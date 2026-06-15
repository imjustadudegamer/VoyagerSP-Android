// sp_bridge.cpp — GetGameAPI bridge: hosts the SP game module (libefgame.so) inside the
// reused HM engine. Fills game_import_t from the engine's real FS/CM/cvar services + a
// minimal CM-backed world, loads a map, and drives ge->Init / ge->RunFrame.
//
// Compiled against the SP game headers (+compat shim). Calls the lilium engine via extern "C"
// declarations whose signatures are ABI-compatible with the SP boundary types (trace_t/vec3_t/
// cvar_t-first-9 all match — see reconciliation research).

#include "q_shared.h"
#include "g_public.h"
#include "tr_types.h"             // glconfig_t, refEntity_t, refdef_t
#include "../cgame/cg_public.h"   // cgameImport_t (CG_*), snapshot_t
#include "../client/vmachine.h"   // cgameExport_t (CG_INIT, CG_DRAW_ACTIVE_FRAME)
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <android/log.h>

// Save-game description typed in the EF Save menu (set by UII_SG_StoreSaveGameComment,
// sp_ui_bridge.cpp). SP_SaveGame writes it into the COMM chunk so the Load list shows a name.
extern char g_spSaveComment[128];

// Com_Printf/Com_Error come via the C shim (SP headers declare them C++-linkage).
extern "C" { void (*SP_ComPrintf(void))(const char*,...); void SP_ComError(int,const char*,...); }
static void blog( const char *fmt, ... ){ va_list a; va_start(a,fmt); __android_log_vprint(ANDROID_LOG_INFO,"EFSP",fmt,a); va_end(a); }

// ---- lilium engine services (resolved at link against the engine objects) ----
extern "C" {
    int   Sys_Milliseconds( void );
    int   Cmd_Argc( void );
    char *Cmd_Argv( int arg );
    void  Cbuf_AddText( const char *text );
    void  CL_SetUserCmdValue( int userCmdValue, float sensitivityScale );  // sets cl.cgameSensitivity (mouse-look scale)
    void  Cmd_TokenizeString( const char *text );
    void  Cmd_AddCommand( const char *cmd_name, void (*function)(void) );
    unsigned Com_BlockChecksum( const void *buffer, int length );   // idTech3 MD4-XOR (retail .sav checksum)
    int   FS_Seek( fileHandle_t f, long offset, int origin );
    int   FS_FOpenFileWrite( const char *qpath );
    int   FS_FOpenFileRead( const char *qpath, fileHandle_t *file, qboolean uniqueFILE );
    void  FS_Rename( const char *from, const char *to );
    cvar_t *Cvar_Get( const char *name, const char *value, int flags );
    void  Cvar_Set( const char *name, const char *value );
    int   Cvar_VariableIntegerValue( const char *name );
    void  Cvar_VariableStringBuffer( const char *name, char *buf, int bufsize );
    char *Cvar_InfoString_Big( int bit );   // all CVAR_ARCHIVE cvars as "\key\val\..." for the CVCN save bag
    int   FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );
    int   FS_Read( void *buffer, int len, fileHandle_t f );
    int   FS_Write( const void *buffer, int len, fileHandle_t f );
    void  FS_FCloseFile( fileHandle_t f );
    long  FS_ReadFile( const char *qpath, void **buffer );
    void  FS_FreeFile( void *buffer );
    int   FS_GetFileList( const char *path, const char *ext, char *listbuf, int bufsize );
    // CM (collision) — model 0 = world BSP
    void  CM_LoadMap( const char *name, qboolean clientload, int *checksum );
    void  CM_ClearMap( void );              // EF1 SP 1:1: paired with Hunk_Clear in SV_SpawnServer
    void  Hunk_Clear( void );               // reclaim the permanent hunk between maps (see SP_FinishTransition)
    char *CM_EntityString( void );
    int   CM_NumInlineModels( void );
    int   CM_InlineModel( int index );
    void  CM_ModelBounds( int model, vec3_t mins, vec3_t maxs );
    int   CM_PointContents( const vec3_t p, int model );
    void  CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
                       const vec3_t mins, const vec3_t maxs, int model, int brushmask, int capsule );
    int   CM_TempBoxModel( const vec3_t mins, const vec3_t maxs, int capsule );
    void  CM_TransformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
                                  const vec3_t mins, const vec3_t maxs, int model, int brushmask,
                                  const vec3_t origin, const vec3_t angles, int capsule );
    int   CM_TransformedPointContents( const vec3_t p, int model, const vec3_t origin, const vec3_t angles );
    int   CM_PointLeafnum( const vec3_t p );
    int   CM_LeafArea( int leafnum );
    int   CM_LeafCluster( int leafnum );
    unsigned char *CM_ClusterPVS( int cluster );
    qboolean CM_AreasConnected( int area1, int area2 );
    void  CM_AdjustAreaPortalState( int area1, int area2, qboolean open );
    void  Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
    void  Cvar_Update( vmCvar_t *vmCvar );
    void  Cmd_ArgsBuffer( char *buffer, int bufferLength );
}

// ---- game module ----
typedef game_export_t* (*GetGameAPI_t)( game_import_t* );
typedef void (*dllEntry_t)( intptr_t (*)(intptr_t,...) );
static void          *g_lib = NULL;
static game_export_t *ge    = NULL;
static int            g_levelTime = 0;
typedef intptr_t (*vmMain_t)( int cmd, ... );
static vmMain_t g_vmMain = NULL;
static int g_spActive = 0;                          // cgame entry point
static intptr_t SP_CgameSyscall( intptr_t cmd, ... );    // cgame->engine dispatcher (defined below)

// ---- minimal server world (entities link here so EntitiesInBox/trace can find them) ----
#define SP_MAX_ENT 1024
static gentity_t *g_linked[SP_MAX_ENT];
static int        g_numLinked = 0;
static char       g_curMap[64] = {0};   // current SP map name (for save framing + transitions)
static int        g_entClip[SP_MAX_ENT];   // bmodel clipHandle per entity number (for mover traces)
static int SP_EntNum( gentity_t *e ){ return (int)(((unsigned char*)e - (unsigned char*)ge->gentities) / ge->gentitySize); }


#define CS_MAX 1024
static char g_configstrings[CS_MAX][1024];

// server->client command channel (subtitles, objectives, centerprints, music...). The game calls
// gi.SendServerCommand; the cgame pulls them by sequence via CG_GETSERVERCOMMAND up to the snapshot's
// serverCommandSequence (cg_servercmds.cpp CG_ExecuteNewServerCommands).
#define SC_RING 256
static char g_svCmds[SC_RING][1024];
static int  g_svCmdSeq = 0;          // latest server command sequence number
static int  g_snapNum   = 1;         // snapshot sequence (declared early: reset in SP_SpawnServer per map)
static int  g_cmdNum    = 1;         // usercmd sequence

// ---- snapshot ring (retail cl.snapshots[PACKET_BACKUP]) + fixed 50ms sim cadence (retail SV_Frame) ----
// Declared early because SP_SpawnServer's per-map reset (below) references them. The ring lets
// CG_GETSNAPSHOT(num) return DISTINCT numbered snapshots so cg.snap (N) and cg.nextSnap (N+1) carry
// serverTimes 50ms apart -> CG_CalcEntityLerpPositions / CG_InterpolatePlayerState interpolate instead of
// teleporting. g_snap (the scratch) is copied into the ring at the end of BuildSnapshot. SNAP_RING power-of-2 >=2.
#define SNAP_RING 4
static snapshot_t g_snapHist[SNAP_RING];
static int        g_snapBaseTime = 0;        // serverTime of snapshot #1 this map (cg.time clamp floor)
#define SP_SV_FRAMEMSEC   50
#define SP_INTERP_DELAY   50                  // trail newest snap by one tick so a nextSnap always exists
#define SP_MAX_CATCHUP    5                   // drain at most 5 ticks (250ms) per render frame (hitch cap)
static int        g_simResidual = 0;          // millisecond accumulator (retail sv.timeResidual)

// ---- savegame: byte-compatible with retail .sav (verified against real PC saves) ----
// chunk on disk = [chid:4 LE][length:4][checksum:4][data:length][magic:4=0x1234abcd]; checksum =
// Com_BlockChecksum (idTech3 MD4-XOR). gi.AppendToSaveGame/ReadFromSaveGame[Optional] operate on the
// one open save file (shared with the engine-side save/load commands via these globals).
static fileHandle_t s_sgFile = 0;
static qboolean     s_sgWriting = qfalse;
#define SG_MAGIC 0x1234abcdu

static gentity_t *SP_GEntity( int num ){ return (gentity_t*)((unsigned char*)ge->gentities + ge->gentitySize*num); }

// ----- world implementations -----
static void W_LinkEntity( gentity_t *ent ){
    // derive absmin/absmax from currentOrigin + mins/maxs
    for(int i=0;i<3;i++){
        ent->absmin[i] = ent->currentOrigin[i] + ent->mins[i] - 1;
        ent->absmax[i] = ent->currentOrigin[i] + ent->maxs[i] + 1;
    }
    // Encode size into entityState.solid for client prediction (mirrors SV_LinkEntity sv_world.c:222-310):
    // SOLID_BMODEL for inline brush models (CG_Mover requires this to pick cgs.inlineDrawModel[] over
    // model_draw[] — else every func_* mover resolves hModel=0 and is invisible); else a packed bbox for
    // solid/body boxes so the client predicts collisions; else 0.
    if( ent->bmodel ) {
        ent->s.solid = SOLID_BMODEL;
    } else if ( ent->contents & ( CONTENTS_SOLID | CONTENTS_BODY ) ) {
        int i = (int)ent->maxs[0];        if(i<1)i=1; if(i>255)i=255;   // x/y assumed symmetric
        int j = (int)(-ent->mins[2]);     if(j<1)j=1; if(j>255)j=255;   // z not symmetric
        int k = (int)(ent->maxs[2]+32);   if(k<1)k=1; if(k>255)k=255;   // z maxs can be negative
        ent->s.solid = (k<<16) | (j<<8) | i;
    } else {
        ent->s.solid = 0;
    }
    // Ensure this entity is in g_linked exactly once. Do NOT trust ->linked alone for membership:
    // after a savegame reload (SP_FinishTransition resets g_numLinked=0) the flag can survive as true
    // while g_linked is empty, so the old `if(!linked)` guard skipped re-adding it. MOVING entities
    // self-heal (G_MoverPush unlinks+relinks every frame), but STATIC ones (triggers like the tutorial's
    // continue_15 lava-cross trigger, static brushes) link once and never re-add -> they silently
    // vanished from EntitiesInBox/W_Trace (no trigger touch, no collision). Search-add (list is small).
    ent->linked = qtrue;
    {
        int li; qboolean present = qfalse;
        for(li=0; li<g_numLinked; li++){ if(g_linked[li]==ent){ present = qtrue; break; } }
        if(!present && g_numLinked < SP_MAX_ENT) g_linked[g_numLinked++] = ent;
    }
}
static void W_UnlinkEntity( gentity_t *ent ){
    if(!ent->linked) return;
    ent->linked = qfalse;
    for(int i=0;i<g_numLinked;i++) if(g_linked[i]==ent){ g_linked[i]=g_linked[--g_numLinked]; break; }
}
static void W_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs,
                     const vec3_t end, int passEntityNum, int contentmask ){
    // 1) world BSP
    CM_BoxTrace( results, start, end, mins, maxs, 0, contentmask, 0 );
    results->entityNum = (results->fraction != 1.0f) ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
    if(results->fraction == 0) return;

    // 2) brush-model movers only — doors / lifts / func_* / force-fields. Iterate the LIVE g_linked
    // list (kept in sync by W_LinkEntity/W_UnlinkEntity). The bmodel filter is cheap; CM_TransformedBoxTrace
    // only runs for the handful of brush models. NOTE: mins/maxs may be NULL (point/line traces:
    // line-of-sight, bullets, the transporter beam) — substitute a zero box or we segfault.
    static const float vz[3] = {0,0,0};
    const float *lmins = mins ? mins : vz, *lmaxs = maxs ? maxs : vz;
    vec3_t tmins, tmaxs;
    for(int k=0;k<3;k++){
        tmins[k] = (start[k]<end[k]?start[k]:end[k]) + lmins[k] - 1;
        tmaxs[k] = (start[k]>end[k]?start[k]:end[k]) + lmaxs[k] + 1;
    }
    // Owner-based skipping (retail SV_ClipMoveToEntities, retail): a trace must ignore not just the
    // pass entity itself but anything OWNED by it (its own projectiles), its OWNER, and siblings sharing that
    // owner. Without this a thrown/fired projectile collides with its shooter the instant it spawns ("detonates
    // on the muzzle") and a player riding a mover he owns self-blocks. EF gentity has `gentity_t *owner`.
    gentity_t* passEnt   = (passEntityNum>=0 && passEntityNum<ge->num_entities) ? SP_GEntity(passEntityNum) : NULL;
    gentity_t* passOwner = passEnt ? passEnt->owner : NULL;
    for(int i=0;i<g_numLinked;i++){
        gentity_t* e = g_linked[i];
        int eNum = SP_EntNum(e);
        if(eNum == passEntityNum || eNum < 0 || eNum >= SP_MAX_ENT) continue;
        if(passEnt){
            if(e->owner == passEnt)                continue;   // entity owned by the pass entity (own projectile)
            if(e == passOwner)                     continue;   // the pass entity's owner
            if(passOwner && e->owner == passOwner) continue;   // sibling sharing the same owner
        }
        if(!(e->contents & contentmask)) continue;
        if(e->absmin[0]>tmaxs[0]||e->absmax[0]<tmins[0]||e->absmin[1]>tmaxs[1]||
           e->absmax[1]<tmins[1]||e->absmin[2]>tmaxs[2]||e->absmax[2]<tmins[2]) continue;
        int h;
        if( e->bmodel ){
            // Brush-model movers (doors/lifts/func_*/force-fields). Derive the clip handle from the
            // entity's OWN inline-model index (like SV_ClipHandleForEntity), NOT the cached g_entClip[]
            // slot which could be stale after a savegame reload -> broad-phase used correct (mins-derived)
            // bounds while the narrow trace hit a DIFFERENT mover -> player passed through lava stones/doors.
            h = ( e->s.modelindex > 0 ) ? CM_InlineModel( e->s.modelindex ) : g_entClip[eNum];
            if(h <= 0) continue;
        } else {
            // Point/bbox solids: misc_model_*_health terminals, point breakables, item-style usables.
            // Retail SV_ClipHandleForEntity builds a TEMP BOX hull from the entity's own mins/maxs so
            // gi.trace can hit them. The bridge previously skipped all non-bmodels, so the cgame crosshair
            // trace (CG_ScanForCrosshairEntity) AND the player's TryUse trace never saw point solids ->
            // no LCARS "use" hint AND the health terminal could not be activated (it fires part3->teleport).
            h = CM_TempBoxModel( e->mins, e->maxs, 0 );
            if(h <= 0) continue;
        }
        trace_t tr;
        // Rotation applies ONLY to brush models (retail retail: non-bmodels pass vec3_origin). A box
        // solid built from mins/maxs is axis-aligned; rotating it by the entity's currentAngles (NPCs face
        // directions) shrinks/rotates the collision box away from its AABB -> bullets/use-traces against a
        // turned NPC or breakable miss or hit at the wrong fraction. Pass angles only for bmodels.
        CM_TransformedBoxTrace(&tr, start, end, lmins, lmaxs, h, contentmask, e->currentOrigin,
                               e->bmodel ? e->currentAngles : vz, 0);
        if(tr.startsolid) results->startsolid = qtrue;
        if(tr.allsolid)   results->allsolid   = qtrue;
        if(tr.fraction < results->fraction){
            qboolean oldStart = results->startsolid;
            tr.entityNum = eNum;
            *results = tr;
            results->startsolid |= oldStart;
        }
    }
}
static int  W_PointContents( const vec3_t p, int passEntityNum ){
    int contents = CM_PointContents(p, 0);   // world model 0
    // Retail SV_PointContents ORs in every overlapping brush-model's real contents. The world-only stub missed
    // water/lava/slime/fog/trigger volumes attached to func_* brush models (moving water, force-fields, content
    // brushes) -> drowning/fog/content-gated triggers on bmodels misbehaved. Walk the linked bmodels.
    for(int i=0;i<g_numLinked;i++){
        gentity_t* e = g_linked[i];
        if(e == NULL || !e->bmodel || e->s.modelindex <= 0) continue;
        if(SP_EntNum(e) == passEntityNum) continue;
        if(p[0]<e->absmin[0]||p[0]>e->absmax[0]||p[1]<e->absmin[1]||p[1]>e->absmax[1]||
           p[2]<e->absmin[2]||p[2]>e->absmax[2]) continue;
        int h = CM_InlineModel(e->s.modelindex);
        if(h<=0) continue;
        contents |= CM_TransformedPointContents(p, h, e->currentOrigin, e->currentAngles);
    }
    return contents;
}
static qboolean W_InPVS( const vec3_t p1, const vec3_t p2 ){
    int l1=CM_PointLeafnum(p1), l2=CM_PointLeafnum(p2);
    unsigned char *vis=CM_ClusterPVS(CM_LeafCluster(l1));
    int c2=CM_LeafCluster(l2);
    if(c2<0) return qfalse;
    return (vis[c2>>3] & (1<<(c2&7))) ? qtrue : qfalse;
}
static void W_SetBrushModel( gentity_t *ent, const char *name ){
    if(!name||name[0]!='*'){ ent->bmodel=qfalse; return; }
    // Real engine SV_SetBrushModel sets s.modelindex = the inline model number; the cgame's CG_Mover
    // indexes cgs.inlineDrawModel[s1->modelindex]. Without this it reads inlineDrawModel[0]=0 -> hModel=0
    // -> CG_Mover early-returns -> EVERY func_* brush-model mover (doors, lava stones, lifts) invisible.
    ent->s.modelindex = atoi(name+1);
    int h = CM_InlineModel( atoi(name+1) );
    CM_ModelBounds( h, ent->mins, ent->maxs );
    ent->bmodel = qtrue;
    ent->contents = -1; // CONTENTS_SOLID-ish; refined later
    int eNum = SP_EntNum(ent);
    if(eNum>=0 && eNum<SP_MAX_ENT) g_entClip[eNum] = h;   // remember the clip handle for W_Trace
    W_LinkEntity(ent);
}
static int  W_EntitiesInBox( const vec3_t mins, const vec3_t maxs, gentity_t **list, int maxcount ){
    int n=0;
    for(int i=0;i<g_numLinked && n<maxcount;i++){
        gentity_t *e=g_linked[i];
        if(e->absmin[0]>maxs[0]||e->absmax[0]<mins[0]) continue;
        if(e->absmin[1]>maxs[1]||e->absmax[1]<mins[1]) continue;
        if(e->absmin[2]>maxs[2]||e->absmax[2]<mins[2]) continue;
        list[n++]=e;
    }
    return n;
}
static qboolean W_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ){
    if(ent->absmin[0]>maxs[0]||ent->absmax[0]<mins[0]) return qfalse;
    if(ent->absmin[1]>maxs[1]||ent->absmax[1]<mins[1]) return qfalse;
    if(ent->absmin[2]>maxs[2]||ent->absmax[2]<mins[2]) return qfalse;
    return qtrue;
}
static void W_AdjustAreaPortalState( gentity_t *ent, qboolean open ){ (void)ent;(void)open; }
static qboolean W_AreasConnected( int a, int b ){ return CM_AreasConnected(a,b); }

// ----- import wrappers (signature/return-type adapters) -----
static int   I_FS_ReadFile( const char *name, void **buf ){ return (int)FS_ReadFile(name,buf); }
static void *I_Malloc( int bytes ){ return malloc((size_t)bytes); }
static void  I_Free( void *p ){ free(p); }
static void  I_SendConsole( const char *t ){
    Cbuf_AddText(t);
}
static void  I_DropClient( int n, const char *r ){ (void)n;(void)r; }
static void  I_SendServerCmd( int n, const char *fmt, ... ){
    (void)n;
    g_svCmdSeq++;
    va_list a; va_start(a,fmt);
    vsnprintf( g_svCmds[g_svCmdSeq & (SC_RING-1)], sizeof(g_svCmds[0]), fmt, a );
    va_end(a);
}
static void  I_SetCS( int n, const char *s ){
    if(n>=0&&n<CS_MAX){
        strncpy(g_configstrings[n], s?s:"", 1023); g_configstrings[n][1023]=0;
        // Mid-game change (CS_MUSIC, objectives, items, CS_MESSAGE...): push a `cs` server command
        // so the cgame updates incrementally (cg_servercmds.cpp `cs` handler). Only AFTER the client
        // is active — at level init the game sets many configstrings that the initial gamestate
        // already carries (BuildGameState), and emitting them all would overflow the command ring.
        if(g_spActive) I_SendServerCmd(0, "cs %i \"%s\"", n, g_configstrings[n]);
    }
}
static void  I_GetCS( int n, char *b, int sz ){ if(n>=0&&n<CS_MAX&&sz>0){ strncpy(b,g_configstrings[n],sz-1); b[sz-1]=0; } else if(sz>0) b[0]=0; }
static void  I_GetUserinfo( int n, char *b, int sz ){
    (void)n; if(sz<=0) return;
    // EF parses SEPARATE headModel/torsoModel/legsModel keys (g_client.cpp ClientUserinfoChanged), NOT a
    // single \model\ key — so \model\munro left all three empty -> no player model -> no mirror reflection.
    // Default loadout = Munro in the hazard suit (Q3_Interface.cpp:2657, g_shared.h DEFAULT_HEADMODEL).
    // Read the cvars so a future scripted disguise change can override; fall back to the retail defaults.
    char head[64]={0}, torso[64]={0}, legs[64]={0};
    Cvar_VariableStringBuffer("headModel",  head,  sizeof(head));
    Cvar_VariableStringBuffer("torsoModel", torso, sizeof(torso));
    Cvar_VariableStringBuffer("legsModel",  legs,  sizeof(legs));
    if(!head[0])  strcpy(head,  "munro/default");
    if(!torso[0]) strcpy(torso, "hazard/default");
    if(!legs[0])  strcpy(legs,  "hazard/default");
    snprintf(b, sz, "\\name\\Munro\\headModel\\%s\\torsoModel\\%s\\legsModel\\%s\\sex\\m\\hc\\200", head, torso, legs);
    b[sz-1]=0;
}
static void  I_SetUserinfo( int n, const char *b ){ (void)n;(void)b; }
static void  I_GetServerinfo( char *b, int sz ){ if(sz>0) b[0]=0; }
// gi.AppendToSaveGame: write one retail-format chunk [id:4][len:4][cksum:4][data][magic:4].
static qboolean I_Append( unsigned long c, void *d, int l ){
    if(!s_sgFile || !s_sgWriting) return qtrue;
    unsigned id=(unsigned)c, len=(unsigned)l, cks=(unsigned)Com_BlockChecksum(d,l), magic=SG_MAGIC;
    FS_Write(&id,4,s_sgFile); FS_Write(&len,4,s_sgFile); FS_Write(&cks,4,s_sgFile);   // exactly 4 bytes each
    if(l>0) FS_Write(d,l,s_sgFile);
    FS_Write(&magic,4,s_sgFile);
    return qtrue;
}
// gi.ReadFromSaveGame[Optional]: read+verify the next chunk; optional = peek-and-skip on id mismatch.
static int ReadChunk( unsigned long c, void *addr, int len, void **pp, qboolean optional ){
    if(!s_sgFile || s_sgWriting) return 0;
    unsigned id=0, storedLen=0;
    if(FS_Read(&id,4,s_sgFile)!=4 || FS_Read(&storedLen,4,s_sgFile)!=4) return 0;
    if(id != (unsigned)c){
        if(optional){ FS_Seek(s_sgFile, -8, FS_SEEK_CUR); return 0; }   // rewind id+len
        // Mandatory mismatch: CONSUME the rest of this chunk (cksum + data + magic) so the stream stays framed
        // and the following chunks still parse, instead of leaving cksum/data/magic unread and desyncing every
        // subsequent chunk (retail Com_Errors here — see stvoy retail; we log + resync to stay non-fatal).
        blog("savegame: chunk ID mismatch got %08x want %08lx (consuming %u + resyncing)\n", id, c, storedLen);
        FS_Seek(s_sgFile, (int)(4 + storedLen + 4), FS_SEEK_CUR);
        return 0;
    }
    if(len != 0 && (unsigned)len != storedLen) blog("savegame: chunk %08x len %u != requested %d\n", id, storedLen, len);
    unsigned cks=0; FS_Read(&cks,4,s_sgFile);
    void *buf = addr;
    if(!buf){ buf = malloc(storedLen ? storedLen : 1); if(pp) *pp = buf; }
    if(storedLen>0) FS_Read(buf, (int)storedLen, s_sgFile);
    unsigned trailer=0; FS_Read(&trailer,4,s_sgFile);
    if((unsigned)Com_BlockChecksum(buf, (int)storedLen) != cks) blog("savegame: checksum FAIL chunk %08x\n", id);
    if(trailer != SG_MAGIC) blog("savegame: bad magic chunk %08x (%08x)\n", id, trailer);
    if(!addr && !pp) free(buf);
    return (int)storedLen;
}
static int I_ReadSG   ( unsigned long c, void *d, int l, void **p ){ return ReadChunk(c,d,l,p,qfalse); }
static int I_ReadSGOpt( unsigned long c, void *d, int l, void **p ){ return ReadChunk(c,d,l,p,qtrue ); }
// Sized to MAX_GENTITIES (1024): the game reads gi.S_Override[ent->s.number] for ANY entity, and
// EF missions have many voiced NPCs at high entity numbers (borg1 alone spawns 561) — a [256] array
// read out of bounds and could stall a character's dialog. Lots of voices/characters need the full range.
static int   g_sOverride[MAX_GENTITIES];
// S_Override voice tracking: the retail engine flags gi.S_Override[ent]=1 while a CHAN_VOICE
// sound plays on that entity, and the game's G_CheckTasksCompleted (g_main.cpp:471) holds the
// TID_CHAN_VOICE task (dialog wait) until it clears. Our engine doesn't, so g_sOverride was
// stuck at 0 and every voice line completed in one frame -> dialog lines fired all at once.
// We approximate it: when a CHAN_VOICE sound starts we record when it should finish, and refresh
// g_sOverride[] from that each frame before ge->RunFrame.
static int   g_voiceEnd[MAX_GENTITIES];
static void SP_UpdateVoiceOverride(void){
    // gi.S_Override[ent] is the facial VISEME index, not a flag: the cgame draws
    // head->customSkin = ci->headSkin + S_Override, where the head ships 8 mouth-shape skins
    // (head_*-1..8) now registered as viseme extensions in the renderer (RE_RegisterSkin). Retail
    // derives the index from the playing CHAN_VOICE sample amplitude; our ioq3 mixer doesn't export
    // that yet, so while a voice line plays we APPROXIMATE lip-sync by cycling the viseme 1..8 at a
    // talking cadence (per-entity phase so characters don't lip-sync in lock-step). 0 = mouth closed
    // (the cgame then runs its own idle blink/frown). This also still gates the dialog `wait` (any
    // nonzero value keeps the TID_CHAN_VOICE task pending until g_voiceEnd elapses).
    for(int i=0;i<MAX_GENTITIES;i++){
        if(g_voiceEnd[i] > g_levelTime){
            unsigned h = (unsigned)(g_levelTime/80 + i*131);   // ~12 Hz, offset per entity
            h = h*1103515245u + 12345u;                        // cheap LCG hash -> natural variation
            g_sOverride[i] = 1 + (int)((h>>16) % 8);           // viseme 1..8 (mouth shapes)
        } else {
            g_sOverride[i] = 0;                                // not talking -> closed/idle
        }
    }
}

// Fully UNLOAD libefgame so the next dlopen returns an OS-zeroed module. Retail reloads the game+cgame DLL
// on every map (FreeLibrary+LoadLibrary), which is what produced "fresh per-map state" — code never bulk-
// memsets cg/cgs/level. This port previously REUSED the .so, so every global the authors assumed the loader
// would zero leaked across missions (player_locked, in_camera, numKnownAnimFileSets, numBoltOns, iCGResetCount,
// ...). libefgame is dlopen'd RTLD_NOW WITHOUT RTLD_GLOBAL, so dlclose truly unloads -> next dlopen = fresh BSS.
// Forward decls for the New-Game-respawn hunk/GPU reclaim (C1) — same proven teardown SP_FinishTransition uses.
extern "C" void SPR_ShutdownRenderer(void);
extern "C" void SPR_InitRenderer(void);
static void     SP_FillGlConfig(void);
// True once an SP world's models/textures occupy the permanent hunk + GPU (set by SP_SpawnServer /
// SP_FinishTransition). SP_SpawnServer consults it to decide whether a New Game must reclaim first (C1).
static qboolean s_worldResident = qfalse;

extern "C" void SP_UnloadGame( void ){
    if( g_lib ){
        if( g_spActive && g_vmMain )            g_vmMain(CG_SHUTDOWN, 0,0,0);   // cgame teardown
        if( g_spActive && ge && ge->Shutdown )  ge->Shutdown();                // game/level teardown
        dlclose( g_lib );
    }
    g_lib = NULL; ge = NULL; g_vmMain = NULL; g_spActive = 0;
    // H3: g_linked[] holds raw gentity_t* into the module we just dlclose'd. Leaving them dangling is a
    // use-after-free waiting for one missing g_spActive gate. The next spawn resets g_numLinked anyway;
    // zero them now so nothing can walk a freed pointer in between.
    g_numLinked = 0; memset(g_linked, 0, sizeof(g_linked));
}

extern "C" qboolean SP_LoadGame( const char *solib ){
    if( g_lib ) SP_UnloadGame();   // drop the stale module first -> every (re)load lands on a fresh, zeroed .so
    g_lib = dlopen( solib, RTLD_NOW );
    if(!g_lib){ blog("SP_LoadGame: dlopen failed: %s\n", dlerror()); return qfalse; }
    GetGameAPI_t GetGameAPI = (GetGameAPI_t)dlsym(g_lib, "_Z10GetGameAPIP13game_import_t");
    if(!GetGameAPI){ blog("SP_LoadGame: GetGameAPI not found\n"); return qfalse; }

    static game_import_t gi; memset(&gi,0,sizeof(gi));
    gi.Printf=SP_ComPrintf(); gi.Error=SP_ComError; gi.Milliseconds=Sys_Milliseconds;
    gi.cvar=Cvar_Get; gi.cvar_set=Cvar_Set;
    gi.Cvar_VariableIntegerValue=Cvar_VariableIntegerValue; gi.Cvar_VariableStringBuffer=Cvar_VariableStringBuffer;
    gi.argc=Cmd_Argc; gi.argv=Cmd_Argv;
    gi.FS_FOpenFile=FS_FOpenFileByMode; gi.FS_Read=FS_Read; gi.FS_Write=FS_Write; gi.FS_FCloseFile=FS_FCloseFile;
    gi.FS_ReadFile=I_FS_ReadFile; gi.FS_FreeFile=FS_FreeFile; gi.FS_GetFileList=FS_GetFileList;
    gi.AppendToSaveGame=I_Append; gi.ReadFromSaveGame=I_ReadSG; gi.ReadFromSaveGameOptional=I_ReadSGOpt;
    gi.SendConsoleCommand=I_SendConsole; gi.DropClient=I_DropClient; gi.SendServerCommand=I_SendServerCmd;
    gi.SetConfigstring=I_SetCS; gi.GetConfigstring=I_GetCS;
    gi.GetUserinfo=I_GetUserinfo; gi.SetUserinfo=I_SetUserinfo; gi.GetServerinfo=I_GetServerinfo;
    gi.SetBrushModel=W_SetBrushModel; gi.trace=W_Trace; gi.pointcontents=W_PointContents;
    gi.inPVS=W_InPVS; gi.inPVSIgnorePortals=W_InPVS;
    gi.AdjustAreaPortalState=W_AdjustAreaPortalState; gi.AreasConnected=W_AreasConnected;
    gi.linkentity=W_LinkEntity; gi.unlinkentity=W_UnlinkEntity;
    gi.EntitiesInBox=W_EntitiesInBox; gi.EntityContact=W_EntityContact;
    gi.S_Override=g_sOverride; gi.Malloc=I_Malloc; gi.Free=I_Free;

    ge = GetGameAPI(&gi);
    if(!ge){ blog("SP_LoadGame: GetGameAPI returned NULL\n"); return qfalse; }
    // wire the cgame syscall dispatcher (no-op) so server-side FX calls don't hit a null pointer
    dllEntry_t dllEntry = (dllEntry_t)dlsym(g_lib, "_Z8dllEntryPFiizE");
    if(dllEntry) dllEntry(SP_CgameSyscall);
    g_vmMain = (vmMain_t)dlsym(g_lib, "_Z6vmMainiiiiiiiii");
    blog("SP_LoadGame: ge=%p apiversion=%d gentitySize=%d\n", (void*)ge, ge->apiversion, ge->gentitySize);
    return (ge->apiversion==GAME_API_VERSION) ? qtrue : qfalse;
}

extern "C" qboolean SP_SpawnServer( const char *mapname ){
    // If a level is already running (New Game / map after a prior map), tear it down FIRST. Re-calling
    // ge->Init without ge->Shutdown leaves stale game state (corrupted level/entity/ICARUS state ->
    // e.g. tutorial scripts/target_scriptrunner failing to spawn). Reuses the loaded libefgame.so.
    // The prior module (if any) was already dlclose'd+reloaded by SP_LoadGame (always called before
    // SP_SpawnServer), so all game/cgame globals are freshly OS-zeroed — no in-place teardown needed.
    char bsp[256]; snprintf(bsp,sizeof(bsp),"maps/%s.bsp", mapname);
    // C1 (unbounded-leak fix): retail SV_SpawnServer reclaims the permanent hunk + collision map on EVERY
    // spawn (Hunk_Clear+CM_ClearMap). The Route-b New-Game path (SP_LoadGame->SP_SpawnServer->SP_StartClient)
    // bypasses SV_SpawnServer and never reclaimed, so starting a New Game while a PRIOR map's models / world /
    // VkImages / descriptor sets were still resident (a replayed New Game, or New Game after playing then
    // returning to the menu) leaked them -> eventual "Hunk_Alloc failed" / VK_ERROR_OUT_OF_POOL_MEMORY.
    // Mirror SP_FinishTransition's proven ordering: GPU teardown FIRST (re.Shutdown walks tr.images[] living
    // in this same perm hunk), THEN Hunk_Clear + CM_ClearMap, THEN re-init the renderer at the end of this
    // function (before SP_StartClient's CG_INIT loads the new world). GATED on s_worldResident: the cold-boot
    // FIRST spawn must NOT do this — there is no prior SP world to reclaim, and an extra R_Init would
    // double-init the descriptor pool on first load (the exact crash documented at SP_StartClient:1048).
    // The prior cgame was already CG_SHUTDOWN'd by SP_LoadGame->SP_UnloadGame before we got here.
    if( s_worldResident ){
        blog("SP_SpawnServer: prior world resident -> reclaiming hunk/GPU before respawn (C1)\n");
        SPR_ShutdownRenderer();   // destroy old textures/imageviews + reset descriptor pool (hunk still valid)
        Hunk_Clear();             // reclaim models/world/backEndData now the GPU side is down
        CM_ClearMap();            // reset cm.name so CM_LoadMap actually reloads
    }
    int checksum=0;
    blog("SP_SpawnServer: CM_LoadMap(%s)\n", bsp);
    CM_LoadMap( bsp, qfalse, &checksum );
    char *entstring = CM_EntityString();
    blog("SP_SpawnServer: %d inline models, entity string %d bytes\n",
               CM_NumInlineModels(), entstring?(int)strlen(entstring):0);
    // EF1 SP 1:1: retail SV_SpawnServer starts the level clock at sv.time = 1000 (retail:30062),
    // NOT 0. Starting at 0 made the opening target_scriptrunner hit the g_target.cpp:707 `level.time<1000`
    // warm-up gate and reschedule to 1000ms, so the cinematic camera (CGCam_Enable -> in_camera) only
    // activated ~1s after the cgame went live -> ~1s of first-person renders first = the Munro/phaser flash
    // at New-Game start. Starting at 1000 fires the opening script/camera BEFORE the first rendered frame,
    // exactly like retail. The +1000ms offset is uniform across the snapshot ring / g_snapBaseTime / cg.time
    // clamps (all relative), so the 20Hz cadence + interpolation are preserved. New-Game-scoped: a LOAD /
    // transition takes g_levelTime from the save (SP_LoadGameFile/SP_FinishTransition), unaffected.
    g_numLinked = 0; g_levelTime = 1000;
    Cvar_Set( "cl_paused", "0" );   // EF1 SP 1:1: retail SV_SpawnServer unpauses every spawn (sv_init.c:464)
    memset(g_voiceEnd, 0, sizeof(g_voiceEnd)); memset(g_sOverride, 0, sizeof(g_sOverride)); // fresh map: no stale voices
    // BRIDGE-side statics dlclose does NOT touch (these live in libmain, not libefgame) — reset per map:
    memset(g_entClip, 0, sizeof(g_entClip)); g_svCmdSeq = 0; g_snapNum = 1; g_cmdNum = 1;
    // Reset the snapshot ring + cadence accumulator so no stale snap from the previous map is served
    // (a stale high-numbered snap would spuriously trip the cgame's CG_RestartLevel / replay old events).
    memset(g_snapHist, 0, sizeof(g_snapHist)); g_snapBaseTime = 0; g_simResidual = 0;
    strncpy(g_curMap, mapname, sizeof(g_curMap)-1); g_curMap[sizeof(g_curMap)-1]=0;
    // Per-map clear of the configstring table (retail SV_SpawnServer memsets the whole table each spawn).
    // g_configstrings lives in libmain and is NOT zeroed by the per-map libefgame dlclose, so without this it
    // accumulates across maps -> model/sound indices drift map-to-map (the borg2 axis-prop class of bug) and
    // dataCount climbs toward MAX_GAMESTATE_CHARS (observed 3133->6656 over borg2->holodeck->voy1). ge->Init
    // below re-registers THIS map's models/sounds fresh. (The LOAD path never comes through SP_SpawnServer —
    // SP_LoadGameFile restores the saved table instead; the transition path clears in SP_FinishTransition.)
    memset(g_configstrings, 0, sizeof(g_configstrings));
    // Populate CS_SERVERINFO so the cgame builds cgs.mapname = "maps/<map>.bsp" (cg_servercmds.cpp:25)
    // — needed for the renderer world load (Route b) AND the cgame levelshot / mission-info text.
    snprintf(g_configstrings[CS_SERVERINFO], sizeof(g_configstrings[0]), "\\mapname\\%s\\sv_maxclients\\1", mapname);
    if(!ge){ blog("no game loaded\n"); return qfalse; }
    ge->Init( mapname, "", checksum, entstring, g_levelTime, Sys_Milliseconds(), Sys_Milliseconds(),
              (SavedGameJustLoaded_e)0, qfalse );
    blog("SP_SpawnServer: ge->Init done. num_entities=%d linked=%d\n", ge->num_entities, g_numLinked);
    // C1 (reclaim path only): bring the renderer we tore down above back up BEFORE SP_StartClient's CG_INIT
    // loads the new world. Mirrors SP_FinishTransition's SP_FillGlConfig()+SPR_InitRenderer() ordering. On the
    // first spawn the boot renderer is already up, so skip (CG_INIT loads the world as before).
    if( s_worldResident ){
        SP_FillGlConfig();
        SPR_InitRenderer();
    }
    s_worldResident = qtrue;
    return qtrue;
}

extern "C" void SP_RunFrames( int n ){
    for(int i=0;i<n;i++){ g_levelTime += 100; SP_UpdateVoiceOverride(); ge->RunFrame(g_levelTime); }
    blog("SP_RunFrames: ticked %d frames to levelTime=%d, num_entities=%d\n", n, g_levelTime, ge->num_entities);
}

// =====================================================================================
//  CLIENT / CGAME side — loopback driver + cgame->engine syscall dispatcher.
//  Renderer/sound are stubbed in this increment (validates cgame loop + struct reconcile
//  headlessly). Real renderervk/sound get wired in the APK phase.
// =====================================================================================
static glconfig_t g_glconfig;
static void SP_FillGlConfig(void);   // fwd: defined near SP_StartClient; called from SP_FinishTransition too
static gameState_t g_gameState;
static snapshot_t  g_snap;
static usercmd_t   g_lastcmd;
static int         g_weaponSelect = 0;

static void BuildGameState(void){
    memset(&g_gameState, 0, sizeof(g_gameState));
    g_gameState.dataCount = 1;   // index 0 reserved (offset 0 = empty)
    for(int i=0;i<CS_MAX;i++){
        const char* s = g_configstrings[i];
        if(!s[0]) continue;
        int len = (int)strlen(s)+1;
        if(g_gameState.dataCount + len >= MAX_GAMESTATE_CHARS) break;
        g_gameState.stringOffsets[i] = g_gameState.dataCount;
        memcpy(g_gameState.stringData + g_gameState.dataCount, s, len);
        g_gameState.dataCount += len;
    }
}

static void BuildSnapshot(void){
    memset(&g_snap, 0, sizeof(g_snap));
    g_snap.serverTime = g_levelTime;
    g_snap.serverCommandSequence = g_svCmdSeq;   // tell the cgame how many server commands exist
    gentity_t* p0 = SP_GEntity(0);
    if(p0->client) g_snap.ps = *(playerState_t*)p0->client;
    int n=0;
    // Start at 0 to INCLUDE the local player's own entity (entity 0). Q3 ships the client's own entity in
    // the snapshot so the cgame can draw the player BODY in mirrors/portals/3rd-person (CG_Player skips it
    // in first-person via clientNum==ps.clientNum). Skipping it (old i=1) meant no Munro reflection at all.
    for(int i=0; i<ge->num_entities && n<MAX_ENTITIES_IN_SNAPSHOT; i++){
        gentity_t* e = SP_GEntity(i);
        if(!e->inuse) continue;
        if(e->svFlags & SVF_NOCLIENT) continue;   // mirror SV_BuildClientSnapshot: don't ship server-only
                                                  // entities; frees slots in the 256 cap so visible movers
                                                  // on dense maps (e.g. borg1 ~561 ents) aren't dropped.
        g_snap.entities[n++] = e->s;
    }
    g_snap.numEntities = n;
    // Publish this snapshot under its number into the ring (retail CL_ParseSnapshot:
    // cl.snapshots[messageNum & PACKET_MASK]). g_snapNum is the number being built NOW.
    g_snapHist[g_snapNum & (SNAP_RING-1)] = g_snap;
    if(g_snapNum == 1) g_snapBaseTime = g_snap.serverTime;   // floor for the cgame's trailing cg.time
    g_snapNum++;
}

#include "sp_render.h"
#define A(n) (args[n])
#define P(n) ((void*)args[n])
#define VMF(n) (*(float*)&args[n])
// Dispatcher for cgame-registered console commands. The SP cgame registers its commands
// (+info/-info mission objectives, give/god/noclip/notarget/undying, setobjective/viewobjective,
// setviewpos, kill, ...) via cgi_AddCommand -> CG_ADDCOMMAND. Each must become a REAL engine
// command that routes back into the cgame's CG_CONSOLE_COMMAND handler (cg_main.cpp) — exactly
// what the stock engine's CL_GameCommand does (cl_cgame.c:1216 VM_Call(cgvm,CG_CONSOLE_COMMAND)).
// The bridge previously stubbed CG_ADDCOMMAND (return 0), so every cgame command was "Unknown
// command" (e.g. pressing +info errored). The engine has already tokenized argv when it runs the
// command, so CG_ConsoleCommand reads it via the bridge's CG_ARGC/CG_ARGV.
static void SP_CgameConsoleCmd(void){
    // CG_CONSOLE_COMMAND returns qtrue if the cgame's local table claimed it (+info, weapnext, zoom...).
    // The SERVER commands the cgame also registers (give/god/noclip/notarget/undying/kill/setviewpos/
    // setobjective/viewobjective, cg_main.cpp:1398-1406) are NOT in that table -> CG_ConsoleCommand returns
    // qfalse, and they must be forwarded to the game's ClientCommand (mirrors retail CL_GameCommand ->
    // CL_ForwardCommandToServer). argv is already tokenized; gi.argv==Cmd_Argv so ClientCommand reads it.
    if(g_vmMain && g_vmMain(CG_CONSOLE_COMMAND, 0,0,0)) return;
    if(ge && g_spActive) ge->ClientCommand(0);
}

// ---------------------------------------------------------------------------
// AS_* ambient-sound-set system (sound/sound.txt). In retail these are ENGINE
// traps (retail: AS_ParseFile retail, S_UpdateAmbientSet retail,
// AS_AddLocalSet retail, AS_GetBModelSound retail). The SP port
// replaced the engine, so they live here. Previously stubbed (return 0/-1) ->
// every map's ambient bed + randomized atmosphere was SILENT. Grammar is a flat
// sequence of blocks (3 set types):
//   <generalSet|localSet|bmodelSet> "<name>" { <keywords> }
//   loopedWave <f>            -> registers sound/<f>.wav        (looping bed)
//   subWaves   <pre> <n>...   -> registers sound/<pre>/<n>.wav  (random one-shots)
//   volRange   <min> <max>    (0..255; advisory, see note)
//   radius     <n>            (engine spatializes by distance already; advisory)
//   timeBetweenWaves <min> <max>  seconds between random subwaves
// Defaults match retail: tbw 10/25 s, volRange 255/255, radius 250. The looped
// bed is anchored on ENTITYNUM_NONE (MAX_GENTITIES-1) — a sentinel the cgame
// never uses for a real entity's loopSound, so it can't collide. Re-added every
// frame because loops are cleared per frame (CG_S_CLEARLOOPINGSOUNDS).
// ---------------------------------------------------------------------------
#define AS_MAX_SETS      512
#define AS_MAX_SUBWAVES  9
struct as_set_t {
    char name[64];
    int  type;                       // 0 general, 1 local, 2 bmodel
    int  tbwMin, tbwMax;             // seconds
    int  volMin, volMax;             // 0..255
    int  radius;
    int  looped;                     // sfxHandle, 0 = none
    int  subWaves[AS_MAX_SUBWAVES];
    int  numSubWaves;
    int  nextWave;                   // realtime ms of next general-set subwave
};
static as_set_t s_asSets[AS_MAX_SETS];
static int      s_asNumSets = 0;
static const float s_asZero[3] = {0,0,0};

// Local string helpers — the engine's q_shared funcs are C-linkage and not visible to this C++ TU.
static int  AS_lc(int c){ return (c>='A'&&c<='Z') ? c+32 : c; }
static int  AS_stricmp(const char* a, const char* b){
    for(;;){ int ca=AS_lc((unsigned char)*a), cb=AS_lc((unsigned char)*b); if(ca!=cb) return ca-cb; if(!ca) return 0; a++; b++; }
}
static void AS_strcpy(char* d, const char* s, int n){ if(n<=0) return; int i=0; for(; i<n-1 && s[i]; i++) d[i]=s[i]; d[i]=0; }
// Self-contained tokenizer (avoids C/C++ linkage mismatch vs the engine's C COM_Parse).
// allowNL=false returns "" at end of line (for the subWaves "<prefix> <name>... EOL" grammar).
// Handles // and /* */ comments, "quoted" set names, and standalone { } tokens.
static char* AS_Tok(char** dp, bool allowNL){
    static char tok[256];
    char* p = *dp; tok[0] = 0;
    if(!p){ return tok; }
    for(;;){
        while(*p && (unsigned char)*p <= ' '){
            if(*p=='\n' && !allowNL){ *dp=p; return tok; }
            p++;
        }
        if(!*p){ *dp=p; return tok; }
        if(p[0]=='/' && p[1]=='/'){ while(*p && *p!='\n') p++; continue; }
        if(p[0]=='/' && p[1]=='*'){ p+=2; while(*p && !(p[0]=='*'&&p[1]=='/')) p++; if(*p) p+=2; continue; }
        break;
    }
    int n=0;
    if(*p=='"'){
        p++;
        while(*p && *p!='"'){ if(n<255) tok[n++]=*p; p++; }
        if(*p=='"') p++;
    } else if(*p=='{' || *p=='}'){
        tok[n++]=*p++;
    } else {
        while(*p && (unsigned char)*p > ' ' && *p!='{' && *p!='}' && *p!='"'){ if(n<255) tok[n++]=*p; p++; }
    }
    tok[n]=0; *dp=p; return tok;
}
static as_set_t* AS_FindSet(const char* name){
    if(!name || !name[0]) return NULL;
    for(int i=0;i<s_asNumSets;i++)
        if(!AS_stricmp(s_asSets[i].name, name)) return &s_asSets[i];
    return NULL;
}
static int AS_RandInterval(const as_set_t* s){
    int span = s->tbwMax - s->tbwMin;
    int secs = s->tbwMin + (span>0 ? (rand()%(span+1)) : 0);
    return secs*1000;
}
static int AS_RandVolume(const as_set_t* s){   // 0-255; retail picks a random subwave volume in [volMin,volMax]
    int span = s->volMax - s->volMin;
    return s->volMin + (span>0 ? (rand()%(span+1)) : 0);
}
static void SP_AS_ParseSets(void){
    s_asNumSets = 0;
    void* buf = NULL;
    int len = (int)FS_ReadFile("sound/sound.txt", &buf);
    if(len <= 0 || !buf){
        blog("AS_ParseSets: ^1Couldn't load ambient sound sets from sound/sound.txt\n");
        return;
    }
    char* p = (char*)buf;
    for(;;){
        const char* tok = AS_Tok(&p,true);
        if(!tok[0]) break;                               // EOF
        int type = -1;
        if(!AS_stricmp(tok,"generalSet"))     type = 0;
        else if(!AS_stricmp(tok,"localSet"))  type = 1;
        else if(!AS_stricmp(tok,"bmodelSet")) type = 2;
        if(type < 0) continue;                           // skip stray token
        char setName[64];
        AS_strcpy(setName, AS_Tok(&p,true), sizeof(setName));
        if(AS_Tok(&p,true)[0] != '{') continue;            // expect '{'
        if(AS_FindSet(setName) || s_asNumSets >= AS_MAX_SETS){
            int depth = 1;                               // dup/full: consume the block
            while(depth){ const char* t=AS_Tok(&p,true); if(!t[0])break; if(t[0]=='{')depth++; else if(t[0]=='}')depth--; }
            continue;
        }
        as_set_t* s = &s_asSets[s_asNumSets++];
        memset(s, 0, sizeof(*s));
        AS_strcpy(s->name, setName, sizeof(s->name));
        s->type=type; s->tbwMin=10; s->tbwMax=25; s->volMin=255; s->volMax=255; s->radius=250;
        for(;;){
            tok = AS_Tok(&p,true);
            if(!tok[0] || tok[0]=='}') break;
            if(!AS_stricmp(tok,"loopedWave")){
                char path[256]; snprintf(path,sizeof(path),"sound/%s.wav",AS_Tok(&p,true));
                s->looped = SPR_S_RegisterSound(path);
                if(s->looped < 1) blog("AS_ParseSets: ^3Unable to load ambient sound \"%s\"\n", path);
            } else if(!AS_stricmp(tok,"subWaves")){
                char prefix[128]; AS_strcpy(prefix, AS_Tok(&p,false), sizeof(prefix));
                const char* wn;
                while((wn=AS_Tok(&p,false))[0]){
                    if(s->numSubWaves >= AS_MAX_SUBWAVES){ blog("AS_ParseSets: ^3Too many subwaves on set \"%s\"\n", s->name); break; }
                    char path[256]; snprintf(path,sizeof(path),"sound/%s/%s.wav",prefix,wn);
                    s->subWaves[s->numSubWaves++] = SPR_S_RegisterSound(path);
                }
            } else if(!AS_stricmp(tok,"volRange")){
                s->volMin = atoi(AS_Tok(&p,false)); s->volMax = atoi(AS_Tok(&p,false));
                if(s->volMin > s->volMax){ int t=s->volMin; s->volMin=s->volMax; s->volMax=t; }
            } else if(!AS_stricmp(tok,"timeBetweenWaves")){
                s->tbwMin = atoi(AS_Tok(&p,false)); s->tbwMax = atoi(AS_Tok(&p,false));
                if(s->tbwMin > s->tbwMax){ int t=s->tbwMin; s->tbwMin=s->tbwMax; s->tbwMax=t; }
            } else if(!AS_stricmp(tok,"radius")){
                s->radius = atoi(AS_Tok(&p,false));
            } else {
                blog("AS_ParseSets: ^3Unknown ambient set keyword \"%s\"\n", tok);
            }
        }
    }
    FS_FreeFile(buf);
    blog("AS_ParseFile: Parsed %d ambient set(s)\n", s_asNumSets);
}
static void SP_AS_UpdateAmbientSet(const char* name, const float* origin){
    as_set_t* s = AS_FindSet(name);
    if(!s) return;
    if(s->looped > 0)
        SPR_S_AddLoopingSound(MAX_GENTITIES-1, origin, s_asZero, s->looped);   // bed on ENTITYNUM_NONE
    if(s->numSubWaves > 0){
        int now = Sys_Milliseconds();
        if(s->nextWave == 0)        s->nextWave = now + AS_RandInterval(s);     // delay first blip
        else if(now >= s->nextWave){
            int h = s->subWaves[rand()%s->numSubWaves];
            if(h > 0) SPR_S_StartSoundVol(NULL, 0, 0 /*CHAN_AUTO*/, h, AS_RandVolume(s));   // non-positional, volRange
            s->nextWave = now + AS_RandInterval(s);
        }
    }
}
static int SP_AS_AddLocalSet(const char* name, const float* origin, int entID, int setTime){
    int now = Sys_Milliseconds();
    as_set_t* s = AS_FindSet(name);
    if(!s) return now;                                                          // retail: "retry next frame"
    if(s->looped > 0 && entID>=0 && entID<MAX_GENTITIES)
        SPR_S_AddLoopingSound(entID, origin, s_asZero, s->looped);
    if(s->numSubWaves > 0 && now >= setTime){
        int h = s->subWaves[rand()%s->numSubWaves];
        if(h > 0) SPR_S_StartSoundVol(origin, entID, 0 /*CHAN_AUTO*/, h, AS_RandVolume(s));   // positional, volRange
        return now + AS_RandInterval(s);
    }
    return setTime;
}
static int SP_AS_GetBModelSound(const char* name, int stage){
    as_set_t* s = AS_FindSet(name);
    if(s && stage>=0 && stage < s->numSubWaves && s->subWaves[stage] > 0) return s->subWaves[stage];
    return -1;                                                                  // sentinel: no sound (door early-out; never a stale 0 handle)
}

static intptr_t SP_CgameSyscall( intptr_t cmd, ... ){
    intptr_t args[16]; va_list ap; va_start(ap,cmd);
    for(int i=0;i<16;i++) args[i]=va_arg(ap,intptr_t);
    va_end(ap);
    switch(cmd){
    case CG_PRINT:        blog("%s",(const char*)P(0)); return 0;
    case CG_ERROR:        SP_ComError(0,"%s",(const char*)P(0)); return 0;
    case CG_MILLISECONDS: return Sys_Milliseconds();
    case CG_CVAR_REGISTER: Cvar_Register((vmCvar_t*)P(0),(const char*)P(1),(const char*)P(2),(int)A(3)); return 0;
    case CG_CVAR_UPDATE:   Cvar_Update((vmCvar_t*)P(0)); return 0;
    case CG_CVAR_SET:      Cvar_Set((const char*)P(0),(const char*)P(1)); return 0;
    case CG_ARGC:          return Cmd_Argc();
    case CG_ARGV:          { char*b=(char*)P(1); int sz=(int)A(2); const char*v=Cmd_Argv((int)A(0)); if(sz>0){strncpy(b,v,sz-1);b[sz-1]=0;} } return 0;
    case CG_ARGS:          Cmd_ArgsBuffer((char*)P(0),(int)A(1)); return 0;
    case CG_FS_FOPENFILE:  return FS_FOpenFileByMode((const char*)P(0),(fileHandle_t*)P(1),(fsMode_t)A(2));
    case CG_FS_READ:       return FS_Read(P(0),(int)A(1),(fileHandle_t)A(2));
    case CG_FS_WRITE:      return FS_Write(P(0),(int)A(1),(fileHandle_t)A(2));
    case CG_FS_FCLOSEFILE: FS_FCloseFile((fileHandle_t)A(0)); return 0;
    case CG_SENDCONSOLECOMMAND: Cbuf_AddText((const char*)P(0)); return 0;
    case CG_ADDCOMMAND:    Cmd_AddCommand((const char*)P(0), SP_CgameConsoleCmd); return 0;
    case CG_SENDCLIENTCOMMAND:
        // The cgame sends SERVER-handled commands (give/god/noclip/notarget/setobjective/...) via
        // trap_SendClientCommand. In our loopback bridge there's no netchan: tokenize the command and
        // hand it straight to the game's ClientCommand (which reads args via gi.argc/argv = Cmd_*),
        // exactly as SP_ClientCommand does. Was stubbed -> these commands silently did nothing.
        if(ge && g_spActive){ Cmd_TokenizeString((const char*)P(0)); ge->ClientCommand(0); }
        return 0;
    case CG_UPDATESCREEN:  return 0;
    // collision -> reuse engine CM (world)
    case CG_CM_LOADMAP:    return 0;   // already loaded server-side
    case CG_CM_NUMINLINEMODELS: return CM_NumInlineModels();
    case CG_CM_INLINEMODEL: return CM_InlineModel((int)A(0));
    case CG_CM_POINTCONTENTS: return CM_PointContents((float*)P(0),(int)A(1));
    case CG_CM_BOXTRACE:   CM_BoxTrace((trace_t*)P(0),(float*)P(1),(float*)P(2),(float*)P(3),(float*)P(4),(int)A(5),(int)A(6),0); return 0;
    case CG_CM_TEMPBOXMODEL: return CM_TempBoxModel((float*)P(0),(float*)P(1),0);
    case CG_CM_TRANSFORMEDPOINTCONTENTS: return CM_TransformedPointContents((float*)P(0),(int)A(1),(float*)P(2),(float*)P(3));
    case CG_CM_TRANSFORMEDBOXTRACE: CM_TransformedBoxTrace((trace_t*)P(0),(float*)P(1),(float*)P(2),(float*)P(3),(float*)P(4),(int)A(5),(int)A(6),(float*)P(7),(float*)P(8),0); return 0;
    case CG_CM_MARKFRAGMENTS: return SPR_MarkFragments((int)A(0),(const void*)P(1),(const float*)P(2),(int)A(3),(float*)P(4),(int)A(5),(void*)P(6));
    case CG_R_DRAWROTATEPIC:  SPR_DrawStretchPic(VMF(0),VMF(1),VMF(2),VMF(3),VMF(4),VMF(5),VMF(6),VMF(7),(int)A(9)); return 0; // rotation VMF(8) dropped
    case CG_R_SCISSOR:        return 0;     // no clip-rect support in re; HUD draws unclipped
    case CG_S_GETSAMPLELENGTH: return SPR_S_GetSampleLength((int)A(0));   // real sound duration ms (subtitle timing)
    // client state (the critical path)
    case CG_GETGLCONFIG:   *(glconfig_t*)P(0) = g_glconfig; return 0;
    case CG_GETGAMESTATE:  *(gameState_t*)P(0) = g_gameState; return 0;
    // Retail CL_GetCurrentSnapshotNumber: return the LATEST built snap's number + its serverTime.
    case CG_GETCURRENTSNAPSHOTNUMBER: {
        int latest = g_snapNum - 1;
        *(int*)P(0) = latest;
        *(int*)P(1) = (latest >= 1) ? g_snapHist[latest & (SNAP_RING-1)].serverTime : g_levelTime;
        return 0;
    }
    // Retail CL_GetSnapshot(num,out): return the ring slot for THAT number; qfalse if too old / future /
    // not yet built (cgame treats qfalse as "extrapolate", NOT an error). Distinct numbered snaps are what
    // give cg.snap != cg.nextSnap (serverTimes 50ms apart) and thus interpolation.
    case CG_GETSNAPSHOT: {
        int num    = (int)A(0);
        int latest = g_snapNum - 1;
        if( num < 1 || num > latest || (latest - num) >= SNAP_RING ) return qfalse;
        *(snapshot_t*)P(1) = g_snapHist[num & (SNAP_RING-1)];
        return qtrue;
    }
    case CG_GETSERVERCOMMAND: {
        int seq = (int)A(0);
        if( seq <= 0 || seq > g_svCmdSeq || seq <= g_svCmdSeq - SC_RING ) return qfalse;
        Cmd_TokenizeString( g_svCmds[seq & (SC_RING-1)] );   // cgame then reads via CG_ARGC/CG_ARGV
        return qtrue;
    }
    case CG_GETCURRENTCMDNUMBER: return g_cmdNum;
    case CG_GETUSERCMD:    *(usercmd_t*)P(1) = g_lastcmd; return qtrue;
    // Weapon select AND the cgame mouse-look sensitivity scale. The 2nd arg (VMF(1)) feeds
    // cl.cgameSensitivity, which CL_MouseMove multiplies the mouse delta by — if left 0 (the
    // CL_ClearState default), touch/mouse look is dead (x0) while key/joystick look still works.
    case CG_SETUSERCMDVALUE: g_weaponSelect=(int)A(0); CL_SetUserCmdValue((int)A(0), VMF(1)); return 0;
    case CG_MEMORY_REMAINING: return 0x4000000;
    // renderer -> real HM renderer via SPR_* wrappers
    case CG_R_REGISTERMODEL:      return SPR_RegisterModel((const char*)P(0));
    case CG_R_REGISTERSKIN:       return SPR_RegisterSkin((const char*)P(0));
    case CG_R_REGISTERSHADER:     return SPR_RegisterShader((const char*)P(0));
    case CG_R_REGISTERSHADERNOMIP:return SPR_RegisterShaderNoMip((const char*)P(0));
    case CG_R_LOADWORLDMAP: {  // Route b: SP loads world; legacy: SPR_LoadWorld no-ops (HM spmap already did)
        const char* wm = (const char*)P(0);
        char fb[128];
        if( !wm || !wm[0] || !strcmp(wm,"maps/.bsp") ){ snprintf(fb,sizeof(fb),"maps/%s.bsp", g_curMap); wm = fb; }  // cgame mapname empty -> use bridge's
        SPR_LoadWorld(wm); blog("CG_R_LOADWORLDMAP %s\n", wm);
        return 0;
    }
    case CG_R_CLEARSCENE:         SPR_ClearScene(); return 0;
    case CG_R_ADDREFENTITYTOSCENE:SPR_AddRefEntity(P(0), (int)sizeof(refEntity_t)); return 0;
    case CG_R_ADDPOLYTOSCENE:     SPR_AddPoly((int)A(0),(int)A(1),P(2)); return 0;
    case CG_R_ADDLIGHTTOSCENE:    SPR_AddLight((const float*)P(0), VMF(1), VMF(2), VMF(3), VMF(4)); return 0;
    case CG_R_RENDERSCENE:        SPR_RenderScene(P(0), (int)sizeof(refdef_t)); return 0;
    case CG_R_SETCOLOR:           SPR_SetColor((const float*)P(0)); return 0;
    case CG_R_DRAWSTRETCHPIC:     SPR_DrawStretchPic(VMF(0),VMF(1),VMF(2),VMF(3),VMF(4),VMF(5),VMF(6),VMF(7),(int)A(8)); return 0;
    case CG_R_MODELBOUNDS:        SPR_ModelBounds((int)A(0),(float*)P(1),(float*)P(2)); return 0;
    case CG_R_LERPTAG:            return SPR_LerpTag(P(0),(int)A(1),(int)A(2),(int)A(3),VMF(4),(const char*)P(5));
    case CG_R_GETLIGHTING:        SPR_GetLighting((const float*)P(0),(float*)P(1),(float*)P(2),(float*)P(3)); return 0;
    case CG_S_REGISTERSOUND:      return SPR_S_RegisterSound((const char*)P(0));
    case CG_S_STARTSOUND: {
        int sEnt=(int)A(1), sChan=(int)A(2), sSfx=(int)A(3);
        SPR_S_StartSound((const float*)P(0), sEnt, sChan, sSfx);
        // CHAN_VOICE=3 / CHAN_VOICE_ATTEN=4 (game channels.h): flag the voice as playing so the
        // dialog script's TID_CHAN_VOICE wait holds until the line finishes (see g_voiceEnd above).
        if((sChan==3 || sChan==4) && sEnt>=0 && sEnt<MAX_GENTITIES){
            int dur = SPR_S_GetSampleLength(sSfx);
            g_voiceEnd[sEnt] = g_levelTime + (dur>0 ? dur : 2500);
            g_sOverride[sEnt] = 1;
        }
        return 0; }
    case CG_S_STARTLOCALSOUND:    SPR_S_StartLocalSound((int)A(0),(int)A(1)); return 0;
    case CG_S_CLEARLOOPINGSOUNDS: SPR_S_ClearLoopingSounds(); return 0;
    case CG_S_ADDLOOPINGSOUND:    SPR_S_AddLoopingSound((int)A(0),(const float*)P(1),(const float*)P(2),(int)A(3)); return 0;
    case CG_S_UPDATEENTITYPOSITION: SPR_S_UpdateEntityPosition((int)A(0),(const float*)P(1)); return 0;
    case CG_S_RESPATIALIZE:       SPR_S_Respatialize((int)A(0),(const float*)P(1),(float(*)[3])P(2),(int)A(3)); return 0;
    case CG_S_STARTBACKGROUNDTRACK: SPR_S_StartBackgroundTrack((const char*)P(0),(const char*)P(1)); return 0;
    // force-feedback (DirectInput joystick rumble; irrelevant on Android) -> no-op
    case CG_FF_STARTFX: case CG_FF_ENSUREFX: case CG_FF_STOPFX: case CG_FF_STOPALLFX: return 0;
    // ambient-sound sets: real AS_* subsystem (sound/sound.txt), ported from retail retail (see above).
    case CG_AS_PARSESETS:         SP_AS_ParseSets(); return 0;
    case CG_AS_ADDENTRY:          return 0;   // precache filter; ParseSets already registers every set's sounds
    case CG_S_UPDATEAMBIENTSET:   SP_AS_UpdateAmbientSet((const char*)P(0),(const float*)P(1)); return 0;
    // args: 0=name 1=listener_origin 2=origin 3=entID 4=time (listener unused; engine spatializes by origin)
    case CG_S_ADDLOCALSET:        return SP_AS_AddLocalSet((const char*)P(0),(const float*)P(2),(int)A(3),(int)A(4));
    // CG_AS_GETBMODELSOUND returns the bmodelSet subwave handle for the given stage, or -1 ("no sound") on miss.
    // -1 (NOT 0) is required: g_mover.cpp G_PlayDoorSound only early-outs on -1; returning 0 would fire
    // G_AddEvent(EV_BMODEL_SOUND, 0) (bogus sfx-handle-0) on every door. Retail: stvoy retail.
    case CG_AS_GETBMODELSOUND: return SP_AS_GetBModelSound((const char*)P(0),(int)A(1));
    case CG_R_DRAWSCREENSHOT: return 0;   // loading-screen savegame thumbnail; re has no capture -> cosmetic
    default: return 0;
    }
}

// ---- savegame file open/close (shared with gi.AppendToSaveGame/ReadFromSaveGame via s_sgFile) ----
static qboolean SG_OpenWrite(const char* path){ s_sgFile = FS_FOpenFileWrite(path); s_sgWriting = qtrue;  return s_sgFile != 0; }
static qboolean SG_OpenRead (const char* path){ fileHandle_t f=0; FS_FOpenFileRead(path,&f,qtrue); s_sgFile=f; s_sgWriting=qfalse; return s_sgFile != 0; }
static void     SG_Close(void){ if(s_sgFile) FS_FCloseFile(s_sgFile); s_sgFile=0; s_sgWriting=qfalse; }

// ---- level transitions (Phase 3): borg1->borg2, turbolift/hub. G_ChangeMap emits maptransition/
// loadtransition; the player's gclient/objectives/ICARUS vars carry via a WriteLevel/ReadLevel save. ----
static char g_transMap[64] = {0}, g_transSpawn[64] = {0};
static int  g_transHub = 0;
// Retail derives the SavedGameJustLoaded flag as (GAME_chunk != 0) + 1: a normal user save writes
// GAME=0 -> eFULL on load; a forward maptransition autosave writes GAME!=0 -> eAUTO on load (eAUTO is
// what makes ClientSpawn run Player_RestoreFromPrevLevel to carry the loadout). We hardcoded eFULL for
// both. Track which path we're on: 1 = forward transition (eAUTO), else the loaded GAME flag.
static int  g_transGameFlag = 0;
// Distinguishes the two eAUTO (g_transGameFlag!=0) paths into SP_FinishTransition, which need OPPOSITE level
// handling: an eAUTO autosave LOAD (death-respawn / `load auto`, set by SP_LoadGameFile) must ge->ReadLevel(qtrue)
// the saved entity-0/objectives/ICARUS; a FORWARD map transition (borg1->borg2, set by SP_MapTransition) must
// DISCARD the snapshot (its __trans.sav holds the OLD map's full table, whose inline-model *N indices would
// crash the new BSP) and carry the player via the playersave/* cvars. 1 = autosave load, 0 = forward transition.
static int  g_transIsAutoLoad = 0;
#define TRANS_SAV "saves/__trans.sav"

// SP loading-screen gate (defined in sp_integration.c): hold the per-map levelshot over the
// whole spmap->efsptrans reload window so the HM loading splash never shows during a transition.
extern "C" void SP_BeginLoad(const char* map);
extern "C" void SP_EndLoad(void);
extern "C" void SPR_LoadWorld(const char* name);   // Route b: SP cgame loads the renderer world (gated on sp_dropHM)
extern "C" void SPR_BeginRegistration(void);       // per-map renderer reset (R_Init) before a reload's CG_INIT
static void SP_WriteEntryAutosave(void);           // level-entry autosave (fresh spawn only) -> death-respawn point
extern "C" void SPR_ShutdownRenderer(void);        // GPU teardown half — MUST run before Hunk_Clear (see below)
extern "C" void SPR_InitRenderer(void);            // R_Init half — runs after Hunk_Clear, before CG_INIT
extern "C" int  SP_DropHM(void);                   // 1 = drop the HM spmap/qagame/cgame (single SP cgame)

// Called from the maptransition/loadtransition command handlers. Saves the live level then reloads
// the world (HM spmap) and finishes the SP re-init (efsptrans) once the new BSP is up.
extern "C" void SP_MapTransition(const char* map, const char* spawn, int isHub){
    if(!ge || !map || !map[0]) return;
    // P1-1: a load/transition is consume-once (g_transMap is cleared only when SP_FinishTransition runs). If
    // one is already pending, a SECOND destination queued in the same Cbuf drain (e.g. a turbolift `use` plus a
    // script-driven changelevel, or a `load` racing a `maptransition`) would silently overwrite g_transMap and
    // drop the first -> wrong-map desync. Ignore the overlapping request; the first transition still completes.
    if( g_transMap[0] ){ blog("SP_MapTransition: '%s' already pending -> ignoring overlapping '%s'\n", g_transMap, map); return; }
    SP_BeginLoad(map);
    blog("SP_MapTransition: -> %s (spawn '%s' hub %d)\n", map, spawn?spawn:"", isHub);
    if(SG_OpenWrite(TRANS_SAV)){ ge->WriteLevel(qfalse); SG_Close(); }   // snapshot carried state
    strncpy(g_transMap, map, sizeof(g_transMap)-1);   g_transMap[sizeof(g_transMap)-1]=0;
    strncpy(g_transSpawn, spawn?spawn:"", sizeof(g_transSpawn)-1); g_transSpawn[sizeof(g_transSpawn)-1]=0;
    g_transHub = isHub;
    g_transGameFlag = 1;   // forward transition -> eAUTO (carry player loadout via Player_RestoreFromPrevLevel)
    g_transIsAutoLoad = 0; // forward transition (NOT an autosave load) -> discard the old-map snapshot below
    char buf[160];
    if( SP_DropHM() ) snprintf(buf,sizeof(buf),"efsptrans\n");                      // Route b: SP_FinishTransition loads world+CM itself
    else              snprintf(buf,sizeof(buf),"spmap %s\nwait 250\nefsptrans\n", map);
    Cbuf_AddText(buf);                 // reload renderer world + CM, then SP_FinishTransition
}

// Called by the 'efsptrans' command after spmap has loaded the new world. Tears down the old SP
// level (reusing the loaded libefgame.so), re-inits on the new map with the carried state.
extern "C" qboolean SP_FinishTransition(void){
    if(!g_transMap[0]){ blog("SP_FinishTransition: no pending map (g_transMap empty) -> abort\n"); return qfalse; }
    // P1-2: silence in-flight one-shots + background music BEFORE the libefgame reload so they don't bleed
    // across the transition. Looping sounds self-heal on the new cgame's first frame (S_ClearLoopingSounds),
    // but one-shots/music have no such reset; without this a VO line or ambient cue carries into the next map.
    { extern void S_StopAllSounds(void); S_StopAllSounds(); }
    // COLD LOAD: if the game module isn't loaded yet, load it now. This happens when a save is loaded
    // straight from the boot main menu with no New Game first — the autoexec no longer auto-loads a map,
    // so libefgame.so was never dlopen'd. Previously ge==NULL made this bail silently, then
    // SP_DropHM_Activate set clc.state=CA_ACTIVE with no game -> client spun (MAX_PACKET_USERCMDS) -> hang.
    SP_LoadGame("libefgame.so");        // UNCONDITIONAL reload: dlclose+dlopen -> OS re-zeroes ALL game+cgame
                                        // globals (retail's per-map DLL reload). s_sgFile is a bridge FS handle,
                                        // so the open save survives -> ge->ReadLevel below restores the saved
                                        // state into the fresh module. SP_UnloadGame already did the teardown.
    if(!ge || !g_vmMain){ blog("SP_FinishTransition: game module failed to load -> abort\n"); return qfalse; }
    blog("SP_FinishTransition: %s spawn=%s\n", g_transMap, g_transSpawn);
    g_spActive = 0;
    memset(g_entClip, 0, sizeof(g_entClip)); g_svCmdSeq = 0; g_snapNum = 1; g_cmdNum = 1;   // bridge statics
    memset(g_snapHist, 0, sizeof(g_snapHist)); g_snapBaseTime = 0; g_simResidual = 0;        // ring + cadence

    // EF1 SP 1:1: retail clears the permanent hunk + collision map on EVERY spawn in SV_SpawnServer
    // (sv_init.c:419-422 — Hunk_Clear() then CM_ClearMap(), before CM_LoadMap). The SP bridge runs the
    // Route-b transition WITHOUT SV_SpawnServer, so nothing reclaimed the hunk: CG_INIT below re-registers
    // every model of the new map into the low hunk (renderervk/tr_model.c R_LoadMD3/MDR -> Hunk_Alloc h_low),
    // and R_Init re-allocs backEndData, on top of every PRIOR map's allocations. After a few transitions the
    // 128MB hunk fills -> "Hunk_Alloc failed" -> recursive error -> crash (observed loading voy3.bsp:651).
    // CM_ClearMap() is required alongside the wipe so CM_LoadMap actually reloads (it resets cm.name, else the
    // same-map fast-path could hand back a map whose data was just cleared).
    //
    // ORDER IS CRITICAL: the renderer's image_t structs (textures of the OLD map — or, on a cold load straight
    // from the boot menu, the MENU's textures) live in this same permanent hunk, and re.Shutdown(qfalse) walks
    // tr.images[] to destroy each VkImageView. So we must tear the GPU resources down FIRST, while that hunk
    // memory is still valid; if Hunk_Clear (and CM_LoadMap reusing the freed space) ran first, re.Shutdown would
    // destroy garbage handles -> vkDestroyImageView SIGSEGV (the load-game crash). R_Init is therefore deferred
    // to SPR_InitRenderer() further below, AFTER the clear, so it re-allocates into the fresh hunk. Nothing
    // renders between here and there, so the renderer sitting shut-down across this span is safe.
    SPR_ShutdownRenderer();   // re.Shutdown(qfalse): destroy old textures/imageviews + reset descriptor pool
    Hunk_Clear();             // reclaim the permanent hunk (models/world/backEndData) now that the GPU side is torn down
    CM_ClearMap();
    char bsp[256]; snprintf(bsp,sizeof(bsp),"maps/%s.bsp", g_transMap);
    int checksum=0; CM_LoadMap(bsp, qfalse, &checksum);
    // Forward transition (no save-table restore): clear the configstring table fresh like retail's per-map
    // spawn, so the new map's model/sound indices are deterministic and the table doesn't grow unbounded across
    // transitions. A LOAD (g_transGameFlag==0) must NOT clear here — SP_LoadGameFile already restored the saved
    // table into g_configstrings (clearing it would drop the restore and reintroduce the index mismatch).
    if( g_transGameFlag != 0 ){
        memset(g_configstrings, 0, sizeof(g_configstrings));
    }
    snprintf(g_configstrings[CS_SERVERINFO], sizeof(g_configstrings[0]), "\\mapname\\%s\\sv_maxclients\\1", g_transMap);  // cgame mapname for Route-b world load + info
    char* entstring = CM_EntityString();
    g_numLinked = 0;
    // eAUTO for forward transitions (so ClientSpawn restores the carried loadout); eFULL for a normal
    // user save-load (unchanged behavior). Mirrors retail SV_ReadGame retail=(GAME!=0)+1.
    SavedGameJustLoaded_e sg = (g_transGameFlag != 0) ? eAUTO : eFULL;
    blog("SP_FinishTransition: sg=%d (transGameFlag=%d hub=%d)\n", (int)sg, g_transGameFlag, g_transHub);
    ge->Init(g_transMap, g_transSpawn, checksum, entstring, g_levelTime,
             Sys_Milliseconds(), Sys_Milliseconds(), sg, g_transHub ? qtrue : qfalse);
    BuildGameState();
    char* denied = ge->ClientConnect(0, qfalse, sg);
    if(denied) blog("SP_FinishTransition: ClientConnect denied: %s\n", denied);
    // Restore carried state BEFORE ClientBegin (retail order: Init -> ClientConnect -> ReadLevel ->
    // ClientBegin). ReadLevel byte-restores every gentity incl. NPC ps.weapon/torsoAnim and reattaches
    // ICARUS; doing it AFTER ClientBegin let the first frame assert PM_TorsoAnimation's armed default
    // (WP_PHASER->TORSO_WEAPONREADY) before the restored calm NPC state was in place -> Tuvok stuck in
    // the phaser-aim pose after a load/transition. For a LOAD, s_sgFile is already open + positioned at
    // the game chunks; for a transition, open __trans.sav now.
    // Forward map transition (eAUTO, g_transGameFlag!=0, e.g. borg1->borg2): borg2's entities were just
    // spawned FRESH by ge->Init above. Do NOT ReadLevel the previous map's saved entity table here — it holds
    // the OLD map's inline brush-model (*N) indices, which are out of range for the new (often smaller) BSP, so
    // the cgame's prediction clip hits CM_InlineModel "bad number" a few frames in -> ERR_DROP -> back to menu.
    // The player's loadout is carried instead via the playersave/playerammo/borgadapt cvars (recorded by the
    // prior level's G_ChangeMap -> Player_CacheForNextLevel) and restored by Player_RestoreFromPrevLevel inside
    // ge->ClientBegin(...,eAUTO) below. (Retail forward transitions also spawn fresh + carry the player via
    // those cvars; they do not restore the old map's entities.) A full USER LOAD (g_transGameFlag==0, same map)
    // still restores the saved entities exactly as before.
    if( g_transGameFlag == 0 ){
        if(s_sgFile || SG_OpenRead(TRANS_SAV)){ s_sgWriting=qfalse; ge->ReadLevel(qfalse, g_transHub ? qtrue : qfalse); SG_Close(); }
    } else if( g_transIsAutoLoad && s_sgFile ){
        // eAUTO autosave LOAD (death-respawn `load *respawn`->auto, or loading a level-entry / genuine retail
        // eAUTO save). The save holds ONLY entity 0 + objectives + ICARUS vars (WriteLevel(qtrue)), so
        // ReadLevel(qtrue) restores those while EVERY other entity respawns FRESH from the BSP (ge->Init above)
        // -> the spent opening script_runner is re-created and the opening cinematic re-fires (re-locking the
        // player), fixing the eFULL free-walk bug (the spent script was restored, so the opening never re-fired).
        // Reading only entity 0 also avoids the forward-transition inline-model crash -- an autosave has no
        // old-map entity table. The
        // player loadout was carried via the CVSV/AMMO/ADPT cvars (restored in SP_LoadGameFile) and is applied
        // by Player_RestoreFromPrevLevel inside ge->ClientBegin(...,eAUTO) below (g_client.cpp:891-896).
        s_sgWriting=qfalse; ge->ReadLevel(qtrue, qfalse); SG_Close();
        blog("SP_FinishTransition: eAUTO autosave load -> ReadLevel(qtrue), entities respawn fresh\n");
    } else if( s_sgFile ){
        SG_Close();   // forward transition (eAUTO): discard the level snapshot; the player carries via cvars
    }
    memset(&g_lastcmd,0,sizeof(g_lastcmd)); g_lastcmd.serverTime = g_levelTime;
    ge->ClientBegin(0, &g_lastcmd, sg);
    // Robustly re-link EVERY in-use brush-model entity after a load. The game's ReadLevel only relinks
    // entities whose SAVED 'linked' flag was true (g_savegame.cpp:790-795); a save made on an older build
    // can bake in linked=false for static brush models, leaving them out of g_linked -> invisible to
    // trigger-touch AND collision (e.g. the tutorial's continue_15 lava-cross trigger + bigforcefield).
    // Moving brush models self-heal (per-frame relink) but static ones never do. Force them all in.
    { int relinked=0;
      for(int i=1;i<ge->num_entities;i++){
          gentity_t* e = SP_GEntity(i);
          if(!e || !e->inuse) continue;
          // Brush-model entities that need to be in g_linked: solid brushes (doors/forcefields/movers,
          // s.solid==SOLID_BMODEL) AND triggers (contents&CONTENTS_TRIGGER 0x40000000 — NOT solid, so they
          // have s.solid!=SOLID_BMODEL and the earlier solid-only pass missed them: that's why the
          // continue_15 trigger stayed unlinked). Add to g_linked using the restored absmin/absmax/contents
          // (don't recompute — ReadLevel already restored them; this is just g_linked membership).
          if(e->s.solid != SOLID_BMODEL && !(e->contents & 0x40000000)) continue;
          e->linked = qtrue;
          { qboolean present=qfalse; int li;
            for(li=0; li<g_numLinked; li++){ if(g_linked[li]==e){ present=qtrue; break; } }
            if(!present && g_numLinked<SP_MAX_ENT){ g_linked[g_numLinked++]=e; relinked++; }
          }
      }
      blog("SP_FinishTransition: re-linked %d brush/trigger entities after ReadLevel (g_numLinked=%d)\n", relinked, g_numLinked);
    }
    SP_UpdateVoiceOverride(); ge->RunFrame(g_levelTime);
    BuildSnapshot();
    BuildGameState();                   // rebuild AFTER ClientBegin so CS_PLAYERS+0 (player model) is in the
                                        // gamestate the cgame reads at CG_INIT (no live CG_CONFIGSTRING path)
    SP_FillGlConfig();                  // MUST run before CG_INIT: without it g_glconfig.vidWidth=0 on the
                                        // LOAD/transition path -> cgame refdef 0x0 -> black 3D world. See helper.
    SPR_InitRenderer();                 // R_Init (the GPU teardown half, SPR_ShutdownRenderer, already ran ABOVE
                                        // before Hunk_Clear). Clears tr.worldMapLoaded so the new world loads
                                        // (fixes "redundantly load world map" on Load Game / level transition).
    g_vmMain(CG_INIT, 0,0,0);           // new cgame (re-registers media + CG_R_LOADWORLDMAP)
    g_spActive = 1;
    s_worldResident = qtrue;            // C1: a world now occupies the hunk/GPU -> a later New Game must reclaim
    // EF1 SP 1:1: retail SV_SpawnServer always unpauses on spawn ("make sure we are not paused",
    // sv_init.c:464 Cvar_Set("cl_paused","0")). The bridge bypasses SV_SpawnServer, so without this a
    // load reached via the in-game ESC menu (UIMENU_INGAME sets cl_paused=1, ui_atoms.c:832) stays
    // paused: SP_MenuPaused() now honors cl_paused (for the inGameCinematic overlay), so the freshly
    // loaded level never advances -> no snapshots -> MAX_PACKET_USERCMDS spin -> hang. Unpause here.
    Cvar_Set( "cl_paused", "0" );
    SP_EndLoad();   // new SP level is live -> stop drawing the transition load screen
    strncpy(g_curMap, g_transMap, sizeof(g_curMap)-1); g_curMap[sizeof(g_curMap)-1]=0;
    g_transMap[0] = 0;
    // EF1 SP 1:1: a FORWARD transition (g_transGameFlag!=0, g_transIsAutoLoad==0, fresh spawn) is a new level
    // entry -> write the level-start eAUTO autosave so death-respawn has a checkpoint. Mirror retail's
    // retail==0 guard (the entry autosave is written only when NO save was just loaded): an eAUTO autosave
    // LOAD (death-respawn, g_transIsAutoLoad==1) must NOT rewrite `auto` -- that would overwrite the player's own
    // checkpoint with the just-reloaded state. A user eFULL LOAD (g_transGameFlag==0) is already excluded.
    if( g_transGameFlag != 0 && !g_transIsAutoLoad ) SP_WriteEntryAutosave();
    blog("SP_FinishTransition: done numEnt=%d ps.origin=(%.0f %.0f %.0f)\n",
         ge->num_entities, g_snap.ps.origin[0], g_snap.ps.origin[1], g_snap.ps.origin[2]);
    return qtrue;
}

// 'use <targetname>' and other game console commands -> the game's ClientCommand (turbolift, triggers).
extern "C" void SP_ClientCommand(void){ if(ge && g_spActive) ge->ClientCommand(0); }

// ---- user save / load (Phase 2). Retail-byte-compatible framing chunks around ge->WriteLevel/ReadLevel.
// Retail's save gate: SV_SaveGame (stvoy retail) calls SV_CanSave (retail), which
// delegates the cutscene check to the game DLL's GameAllowedToSaveHere() (game_export +0x14 == !in_camera).
// The port already ships that export (g_savegame.cpp:999) but never consulted it. Expose it for the UI
// bridge (sp_ui_bridge.cpp) and the console-save backstop below. Returns true if no game is loaded.
extern "C" qboolean SP_GameAllowedToSaveHere(void){
    return (ge && ge->GameAllowedToSaveHere) ? ge->GameAllowedToSaveHere() : qtrue;
}

// Set when an AUTOSAVE was requested during a cutscene (in_camera): SP_DrawFrame writes it on the first
// non-cinematic frame. See SP_SaveGame / the death-respawn-into-cutscene fix below.
static int g_pendingAutosave = 0;

// Internal save writer. bEAUTO selects retail SV_SaveGame's two flavours (stvoy retail/retail):
//  - bEAUTO=qfalse -> eFULL: GAME=0 + ge->WriteLevel(qfalse) (full gclient + entity table). User saves and the
//    mid-level target_autosave checkpoint (`save auto*`, retail gameFlag 0). On load the exact level is restored.
//  - bEAUTO=qtrue  -> eAUTO: GAME=1 + ge->WriteLevel(qtrue) (entity 0 + objectives + ICARUS only) + the player
//    carry chunks CVSV/AMMO/ADPT. The level-entry autosave (retail writes it with gameFlag 1, decomp 0x424f36).
//    On load the level respawns FRESH from the BSP (the spent opening script_runner re-fires -> the cinematic
//    re-locks the player) and the loadout is restored via Player_RestoreFromPrevLevel from the carry cvars.
static void SP_SaveGameInternal(const char* name, qboolean bEAUTO){
    if(!ge || !g_spActive || !name || !name[0]){ blog("save: not in a game\n"); return; }
    // Cutscene gate (retail SV_CanSave -> GameAllowedToSaveHere == !in_camera). Retail retail only checks
    // it for gameFlag 0: `if ((param_2==0) && !SV_CanSave) return;`. So an eAUTO autosave (bEAUTO) is NEVER
    // gated -- it writes immediately even during the opening cinematic, which is SAFE now that it is eAUTO:
    // WriteLevel(qtrue) records no spent-script/in_camera entity state, and the load respawns fresh (the
    // cutscene replays). Only eFULL saves honour the gate:
    //  - MANUAL eFULL save during a cutscene: REFUSE (matches retail).
    //  - eFULL "auto" (target_autosave checkpoint) during a cutscene: DEFER to the first non-cinematic frame.
    qboolean isAuto = (strcmp(name,"auto") == 0) ? qtrue : qfalse;
    if( !bEAUTO && ge->GameAllowedToSaveHere && !ge->GameAllowedToSaveHere() ){
        if( isAuto ){
            g_pendingAutosave = 1;
            blog("save: eFULL autosave checkpoint DEFERRED (in cutscene) -> will write when in_camera clears\n");
        } else {
            SP_ComPrintf()("^1Cannot save during a cutscene.\n");   // engine console (Com_Printf is C-linkage; route via the shim)
            blog("save: manual save blocked during cutscene -- matches retail SV_CanSave\n");
        }
        return;
    }
    if( isAuto ) g_pendingAutosave = 0;   // writing an autosave now satisfies any pending deferral
    if(!SG_OpenWrite("saves/current.sav")){ blog("save: open failed\n"); return; }
    static unsigned char shot[256*256*4];      // SHOT thumbnail: 256x256 RGBA, bottom-up (retail 0x40000-byte chunk)
    char comm[128], mpcm[1024]; int zero=0, tnow=g_levelTime;
    memset(comm,0,sizeof(comm)); memset(mpcm,0,sizeof(mpcm));
    // EFSP: embed a real screenshot. SPR_GetSaveThumb copies the latest framebuffer capture (armed ~1x/sec on
    // un-paused gameplay frames in SP_DrawFrame, so it's the clean 3D+HUD with NO menu overlay — the ESC/Save
    // menu pauses, freezing further captures, so the last clean game frame is what a manual save embeds).
    // Falls back to black if nothing has been captured yet (e.g. a save before the first gameplay frame).
    if( !SPR_GetSaveThumb(shot) ) memset(shot,0,sizeof(shot));
    // COMM (128) = the save comment the Load/Save menu parses (ui_game.cpp:617-643, iSG_COMMENT_SIZE):
    //   [0..62] description, [64..71] time, [73..82] date, [84..] map name. Each null-terminated.
    {
        const char* desc = g_spSaveComment[0] ? g_spSaveComment : g_curMap;
        // Autosaves funnel through SP_SaveGame("auto") (level-entry, target_autosave's `save auto*`,
        // and forward-transition autosave). Label them so the Load menu reads "Autosave - <map>"
        // instead of a bare mapname; ignore any stale user comment for the auto slot.
        char autobuf[64];
        if( !strcmp(name, "auto") ){          // name is the literal lowercase "auto" for every autosave path
            snprintf(autobuf, sizeof(autobuf), "Autosave - %s", g_curMap);
            desc = autobuf;
        }
        int i; for(i=0;i<63 && desc[i];i++) comm[i]=desc[i];
        time_t tt = time(NULL); struct tm* lt = localtime(&tt);
        if(lt){ strftime(comm+64, 9, "%H:%M:%S", lt); strftime(comm+73, 9, "%m/%d/%y", lt); }   // retail uses 2-digit year "MM/DD/YY" (stvoy retail)
        for(i=0;i<43 && g_curMap[i];i++) comm[84+i]=g_curMap[i];
    }
    snprintf(mpcm,sizeof(mpcm),"%s", g_curMap);
    I_Append('COMM', comm, 128); I_Append('SHOT', shot, 256*256*4); I_Append('MPCM', mpcm, 1024);
    // CVCN: persist EVERY CVAR_ARCHIVE cvar so a load restores the exact save-time state, byte-matching retail
    // (stvoy retail walks the cvar registry and writes a 'CVCN' count then ('CVAR' name)+('VALU' value)
    // pairs for each archived cvar; a retail auto.sav carries g_gravity/legsModel/torsoModel/headModel/handicap/
    // sex/snaps/name/g_spskill). g_spskill (difficulty) is the gameplay-critical one; the rest restore the
    // player's model/name/handicap to save-time values. Read back generically by SP_LoadGameFile.
    {
        char info[8192]; { const char* src=Cvar_InfoString_Big(CVAR_ARCHIVE); int i=0; for(; src[i] && i<8191; i++) info[i]=src[i]; info[i]=0; }
        int seps=0; for(const char* c=info; *c; c++) if(*c=='\\') seps++;
        int nc = seps/2;                                     // infostring "\k\v\k\v" => 2 backslashes per cvar
        I_Append('CVCN', &nc, 4);
        const char* p = info;
        for(int ci=0; ci<nc; ci++){
            char nm[64]={0}, vl[256]={0}; int n;
            if(*p=='\\') p++;  n=0; while(*p && *p!='\\' && n<63)  nm[n++]=*p++; nm[n]=0;   // key
            if(*p=='\\') p++;  n=0; while(*p && *p!='\\' && n<255) vl[n++]=*p++; vl[n]=0;   // value
            I_Append('CVAR', nm, (int)strlen(nm)+1);
            I_Append('VALU', vl, (int)strlen(vl)+1);
        }
    }
    int gameFlag = bEAUTO ? 1 : 0;
    I_Append('GAME', &gameFlag, 4);
    // eAUTO carry (retail retail, written ONLY when gameFlag!=0): persist the engine carry-cvars the
    // game's Player_RestoreFromPrevLevel reads on the fresh respawn. The cvar names MUST match the game exactly
    // -- "playersave"/"playerammo%d"/"borgadapt%d", NO underscore (g_client.cpp:698/712/719). Written right
    // after GAME so SP_LoadGameFile's game!=0 branch consumes them in the SAME order (the stream stays framed).
    // eFULL saves write none of these -> their load (game==0) reads none -> existing saves are unaffected.
    if( bEAUTO ){
        char cv[1024];
        cv[0]=0; Cvar_VariableStringBuffer("playersave", cv, sizeof(cv)); I_Append('CVSV', cv, (int)strlen(cv)+1);
        for(int i=0;i<4;i++){  char nm[32]; snprintf(nm,sizeof(nm),"playerammo%d",i); cv[0]=0; Cvar_VariableStringBuffer(nm,cv,sizeof(cv)); I_Append('AMMO', cv, (int)strlen(cv)+1); }
        for(int i=0;i<32;i++){ char nm[32]; snprintf(nm,sizeof(nm),"borgadapt%d",i); cv[0]=0; Cvar_VariableStringBuffer(nm,cv,sizeof(cv)); I_Append('ADPT', cv, (int)strlen(cv)+1); }
    }
    I_Append('TIME', &tnow, 4);
    I_Append('TIMR', &zero, 4);
    // PRTS: retail emits a particle/precip grid chunk here, between TIMR and CSCN (stvoy retail). The port
    // doesn't simulate that grid (particles reseed on spawn), but write a zero PRTS so the chunk stream is
    // structurally retail-identical and frames the same on both sides; SP_LoadGameFile read-skips it.
    { static unsigned char prts[1296]; I_Append('PRTS', prts, (int)sizeof(prts)); }
    // CSCN/CSIN/CSDA: persist the configstring table so a load can rebuild the EXACT index->string map that
    // every entity's s.modelindex (and the cgame's model_draw[]/sound indices) were assigned against. The
    // bridge's g_configstrings lives in libmain and is NEVER cleared per-map, so a load that doesn't restore
    // it leaves the cgame's rebuilt table mismatched against the ReadLevel'd entity indices -> wrong/zero
    // model handles (borg2 props drew the RGB null-model axis because a saved s.modelindex landed on an empty
    // _c1 chunk slot in the now-stale table). Mirrors retail retailxx (skip index 1 = systeminfo).
    { int csn=0;
      for(int i=0;i<CS_MAX;i++){ if(i!=1 && g_configstrings[i][0]) csn++; }
      I_Append('CSCN', &csn, 4);
      for(int i=0;i<CS_MAX;i++){
          if(i==1 || !g_configstrings[i][0]) continue;
          I_Append('CSIN', &i, 4);
          I_Append('CSDA', g_configstrings[i], (int)strlen(g_configstrings[i])+1);
      } }
    ge->WriteLevel(bEAUTO);                     // eFULL(qfalse): gclient + full entity table; eAUTO(qtrue): entity 0 + objectives/ICARUS only
    SG_Close();
    char to[96]; snprintf(to,sizeof(to),"saves/%s.sav", name);
    FS_Rename("saves/current.sav", to);
    blog("save: wrote %s (map %s, t=%d, %s)\n", to, g_curMap, tnow, bEAUTO ? "eAUTO" : "eFULL");
}

// Public save entry (the `save` console command, deferred-autosave writer, quick-save): always eFULL, exactly
// like retail's `save`/`wipe` command path (gameFlag 0). The eAUTO level-entry autosave goes through
// SP_SaveGameInternal(...,qtrue) from SP_WriteEntryAutosave only.
extern "C" void SP_SaveGame(const char* name){ SP_SaveGameInternal(name, qfalse); }

// EF1 SP 1:1: retail writes an `auto` autosave on every FRESH (non-loaded) level entry, right after the
// first ClientBegin (stvoy SV spawn path), so the death-screen respawn (`load *respawn` -> load `auto`,
// see SP_Load_f) ALWAYS has a level-start checkpoint to reload -- even before the level's first
// target_autosave. Holodeck/_brig (`_`-prefixed) maps are skipped, exactly like retail (respawn() spawns
// in place there). Called from the fresh-spawn paths only (New Game, forward transition) -- NEVER on a user
// load, which would clobber the player's own checkpoint with the just-loaded state.
static void SP_SaveGameInternal(const char* name, qboolean bEAUTO);   // fwd
static void SP_WriteEntryAutosave(void){
    if( !g_spActive ) return;
    if( g_curMap[0] == '_' ) return;        // _holo*/_brig: no entry autosave (respawn() spawns in place)
    // Retail writes the level-entry "auto" with gameFlag 1 = eAUTO (stvoy decomp 0x424f36: retail("auto",1)
    // on a fresh non-'_' map entry, retail==0). eAUTO is what makes the death-respawn `load *respawn`
    // re-spawn the level FRESH (opening cinematic re-fires + re-locks the player) instead of restoring the
    // spent opening script_runner (the eFULL free-walk bug, where the cinematic resumed but the player was free).
    SP_SaveGameInternal("auto", qtrue);
    blog("SP_WriteEntryAutosave: wrote eAUTO level-entry autosave for %s\n", g_curMap);
}

extern "C" void SP_LoadGameFile(const char* name){
    if(!name || !name[0]) return;
    // P1-1: don't clobber an already-pending load/transition (consume-once g_transMap). A load requested while
    // another is mid-flight would overwrite the pending map + reuse __trans.sav -> wrong-map desync.
    if( g_transMap[0] ){ blog("SP_LoadGameFile: '%s' already pending -> ignoring overlapping load '%s'\n", g_transMap, name); return; }
    char path[96]; snprintf(path,sizeof(path),"saves/%s.sav", name);
    if(!SG_OpenRead(path)){ blog("load: open failed %s\n", path); return; }
    static unsigned char shot[256*256*4];
    char comm[128], mpcm[1024]; int ncvc=0, game=0, tnow=0, timr=0, cscn=0;
    // EFSP 4:3: cg_forceAspect is a DISPLAY preference (the video-menu Aspect toggle), not gameplay state. The
    // CVCN bag below restores EVERY saved CVAR_ARCHIVE cvar verbatim (retail SV_ReadGame — left byte-identical),
    // and old saves baked in whatever aspect was set when they were written. We do NOT special-case the retail
    // restore loop; instead we snapshot the user's CURRENT aspect here and re-assert it AFTER the loop, so a
    // load never changes what's on screen (matches "persist unless manually changed"). cg_forceAspect doesn't
    // exist in retail saves anyway, so this is purely a port display-layer guard around an unmodified save read.
    char aspectPref[16]; aspectPref[0]=0; Cvar_VariableStringBuffer("cg_forceAspect", aspectPref, sizeof(aspectPref));
    I_ReadSG('COMM', comm,128,0); I_ReadSG('SHOT', shot,256*256*4,0); I_ReadSG('MPCM', mpcm,1024,0);
    I_ReadSG('CVCN', &ncvc,4,0);                              // M-1: retail's archived-cvar count FourCC (was 'NCVC')
    for(int i=0;i<ncvc;i++){
        char cv[64], vl[256]; cv[0]=0; vl[0]=0;
        I_ReadSG('CVAR',cv,0,0); I_ReadSG('VALU',vl,0,0);
        cv[sizeof(cv)-1]=0; vl[sizeof(vl)-1]=0;
        // restore each saved archived cvar (g_spskill/difficulty, model/name/handicap) BEFORE the deferred
        // efsptrans spawns the map, so the game reads save-time state at ge->Init, exactly like retail SV_ReadGame.
        if(cv[0]) Cvar_Set(cv, vl);
    }
    // EFSP 4:3: re-assert the user's live aspect preference over whatever the save restored above (display
    // preference, not save state). The retail restore loop ran untouched; this just keeps the screen aspect
    // unchanged across a load. Empty pref (cvar somehow absent) -> leave the save's value alone.
    if(aspectPref[0]) Cvar_Set("cg_forceAspect", aspectPref);
    I_ReadSG('GAME',&game,4,0);
    // L-3: a retail eAUTO save (GAME!=0) carries the cross-level player loadout here — CVSV(playersave, 1024) +
    // 4x AMMO(playerammo_%d) + 32x ADPT(borgadapt_%d) (stvoy retail). Consume + Cvar_Set so the stream
    // stays framed and the carry state restores. Port-written saves are always eFULL (GAME==0), so this is only
    // exercised when loading a genuine retail transition autosave.
    if(game!=0){
        // Restore the eAUTO carry cvars the game's Player_RestoreFromPrevLevel reads on the fresh respawn.
        // The cvar names MUST match the game EXACTLY -- "playerammo%d"/"borgadapt%d" with NO underscore
        // (g_client.cpp:662/669; retail efgame strings 200ce008/200cdffc). The earlier "playerammo_%d"/
        // "borgadapt_%d" set cvars the game never reads -> ammo/borg-adapt silently never restored. (Only ever
        // reachable for game!=0, i.e. eAUTO saves; eFULL saves skip this whole block, so older saves are
        // unaffected.) CVSV read len 0 = take the stored size (the write side is strlen+1).
        char cvsv[1024]; cvsv[0]=0; I_ReadSG('CVSV', cvsv,0,0); cvsv[sizeof(cvsv)-1]=0; Cvar_Set("playersave", cvsv);
        for(int i=0;i<4;i++){  char a[512]; a[0]=0; I_ReadSG('AMMO', a,0,0); a[sizeof(a)-1]=0; char nm[32]; snprintf(nm,sizeof(nm),"playerammo%d",i); Cvar_Set(nm,a); }
        for(int i=0;i<32;i++){ char d[512]; d[0]=0; I_ReadSG('ADPT', d,0,0); d[sizeof(d)-1]=0; char nm[32]; snprintf(nm,sizeof(nm),"borgadapt%d",i); Cvar_Set(nm,d); }
    }
    I_ReadSG('TIME',&tnow,4,0); I_ReadSG('TIMR',&timr,4,0);
    { void* pr=0; if(I_ReadSGOpt('PRTS',0,0,&pr)>0 && pr) free(pr); }   // L-1: consume retail's particle-grid chunk if present
    I_ReadSG('CSCN',&cscn,4,0);
    // Restore the saved configstring table (retail retail: clear the whole table, then write each saved
    // index->string back). The cgame's model_draw[]/sound indices and every entity's s.modelindex were assigned
    // against THIS table at save time; g_configstrings (libmain) is never cleared per-map, so without this the
    // table BuildGameState rebuilds no longer matches the restored entity modelindex -> wrong/zero handles
    // (borg2's missing props / RGB null-model axis: a saved modelindex landed on an empty _c1 chunk slot).
    // Restoring here (before SP_FinishTransition's ge->Init) makes G_ModelIndex re-find the saved indices, so
    // the fresh spawn, the ReadLevel'd entities, and the cgame gamestate all agree. cscn==0 (pre-fix saves)
    // simply clears -> those legacy saves can't be recovered (the table was never written), but new saves are.
    for(int i=0;i<CS_MAX;i++) g_configstrings[i][0]=0;
    for(int i=0;i<cscn;i++){
        int idx=-1; char dat[1024]; dat[0]=0;
        I_ReadSG('CSIN',&idx,4,0);
        if(idx>=0 && idx<CS_MAX){ I_ReadSG('CSDA', g_configstrings[idx], 0, 0); g_configstrings[idx][sizeof(g_configstrings[0])-1]=0; }
        else                     I_ReadSG('CSDA', dat, 0, 0);   // out-of-range index: consume the chunk + discard
    }
    // s_sgFile stays OPEN here, positioned at the WriteLevel game chunks -> SP_FinishTransition ReadLevels it.
    strncpy(g_transMap, mpcm, sizeof(g_transMap)-1); g_transMap[sizeof(g_transMap)-1]=0;
    g_transSpawn[0]=0; g_transHub=0; g_levelTime = tnow;
    g_transGameFlag = game;   // (GAME!=0)+1 -> eAUTO for transition-saves, eFULL for normal user saves (GAME=0)
    g_transIsAutoLoad = (game != 0) ? 1 : 0;   // an eAUTO LOAD (vs forward transition): ReadLevel(qtrue) below
    SP_BeginLoad(mpcm);   // levelshot of the save's map over the reload window
    char buf[160];
    if( SP_DropHM() ) snprintf(buf,sizeof(buf),"efsptrans\n");                       // Route b: no HM spmap
    else              snprintf(buf,sizeof(buf),"spmap %s\nwait 250\nefsptrans\n", mpcm);
    Cbuf_AddText(buf);
    blog("load: %s -> map %s t=%d (deferred reload)\n", path, mpcm, tnow);
}

// Populate the cgame's glconfig (returned to the cgame via CG_GETGLCONFIG). CG_CalcVrect derives
// refdef.width/height from vidWidth/vidHeight, so if these are 0 the 3D viewport is 0x0 and R_RenderView
// early-returns (tr_main.c) -> the world renders BLACK while 2D/HUD/menu (independent of vidWidth) draw
// fine. This MUST run before CG_INIT on EVERY path that brings up a cgame. It used to live only in
// SP_StartClient (New Game); the LOAD / map-transition path (SP_FinishTransition) skipped it, so a
// cold-boot Load Game left g_glconfig.vidWidth=0 -> black 3D.
static void SP_FillGlConfig(void){
    memset(&g_glconfig,0,sizeof(g_glconfig));
    int rw=640, rh=480; SPR_GetVidSize(&rw,&rh); if(rw<=0)rw=640; if(rh<=0)rh=480;
    g_glconfig.vidWidth=rw; g_glconfig.vidHeight=rh; g_glconfig.windowAspect=(float)rw/(float)rh;
    blog("SP_FillGlConfig: vid size %dx%d\n", rw, rh);
    strcpy(g_glconfig.renderer_string,"efsp-headless");
    strcpy(g_glconfig.vendor_string,"efsp");
    strcpy(g_glconfig.version_string,"1.0");
}

extern "C" qboolean SP_StartClient(void){
    if(!ge||!g_vmMain){ blog("SP_StartClient: no game/cgame\n"); return qfalse; }
    // New Game is a FRESH start with NO carried-in loadout. Clear the cross-level carry cvars
    // (playersave/playerammo%d/borgadapt%d) that a prior same-session playthrough's forward transitions left set
    // (they are non-archived, so a cold app launch already has them empty -- this covers play->menu->New Game).
    // Otherwise the eAUTO level-entry autosave written below would serialize that STALE loadout into its
    // CVSV/AMMO/ADPT chunks, and a death-respawn on this New Game would restore the previous game's weapons/health.
    // Empty carry -> Player_RestoreFromPrevLevel no-ops (it gates on strlen(playersave)) -> the default ClientSpawn
    // loadout, which is the correct New-Game respawn. Forward transitions/loads set these elsewhere and are unaffected.
    Cvar_Set("playersave", "");
    for(int i=0;i<4;i++){  char nm[32]; snprintf(nm,sizeof(nm),"playerammo%d",i); Cvar_Set(nm,""); }
    for(int i=0;i<32;i++){ char nm[32]; snprintf(nm,sizeof(nm),"borgadapt%d",i); Cvar_Set(nm,""); }
    SP_FillGlConfig();   // glconfig (vidWidth/Height) for the cgame's CG_CalcVrect — see SP_FillGlConfig
    BuildGameState();
    // connect + spawn the single player
    char* denied = ge->ClientConnect(0, qtrue, (SavedGameJustLoaded_e)0);
    if(denied){ blog("SP_StartClient: ClientConnect denied: %s\n", denied); return qfalse; }
    memset(&g_lastcmd,0,sizeof(g_lastcmd)); g_lastcmd.serverTime=g_levelTime;
    ge->ClientBegin(0, &g_lastcmd, (SavedGameJustLoaded_e)0);
    blog("SP_StartClient: client connected+begun\n");
    SP_UpdateVoiceOverride(); ge->RunFrame(g_levelTime);
    BuildSnapshot();
    // Rebuild the gamestate AFTER ClientBegin/RunFrame so it includes configstrings the game set during
    // spawn — notably CS_PLAYERS+0 (the local player's model/skin). The cgame reads configstrings ONLY
    // from CG_GETGAMESTATE (no live CG_CONFIGSTRING), so without this the player has no clientInfo:
    // no Munro model in mirrors/portals and no 3rd-person body. (Was built before ClientConnect -> empty.)
    BuildGameState();
    blog("SP_StartClient: first snapshot numEntities=%d ps.origin=(%.0f %.0f %.0f) ps.weapon=%d\n",
         g_snap.numEntities, g_snap.ps.origin[0], g_snap.ps.origin[1], g_snap.ps.origin[2], g_snap.ps.weapon);
    blog("SP_StartClient: cgame CG_INIT...\n");
    // Do NOT call SPR_BeginRegistration()/CL_InitRenderer here. On the fresh New-Game path there is no prior
    // cgame to CG_SHUTDOWN first, so an extra R_Init re-registers all renderer media WITHOUT releasing the
    // previous descriptor sets -> the Vulkan descriptor pool (vk.storage.descriptor) is exhausted on the
    // FIRST load (VK_ERROR_OUT_OF_POOL_MEMORY). The "redundantly load world map" error only occurs on a
    // *repeated* load of the SAME map, which is now prevented at the source by the SP_MapWrap_f debounce.
    // (SP_FinishTransition resets safely because it CG_SHUTDOWNs the prior cgame first, freeing those sets.)
    g_vmMain(CG_INIT, 0, 0, 0);
    blog("SP_StartClient: CG_INIT returned OK\n");
    g_spActive = 1;
    SP_EndLoad();   // SP cgame is live now -> SP_DrawFrame owns the screen; stop the load screen
    SP_WriteEntryAutosave();   // New Game = fresh entry -> level-start autosave for death-respawn
    return qtrue;
}

extern "C" void SP_ClientFrames(int n){
    for(int i=0;i<n;i++){
        g_levelTime += 50;
        g_lastcmd.serverTime = g_levelTime;
        g_cmdNum++;
        ge->ClientThink(0, &g_lastcmd);
        SP_UpdateVoiceOverride(); ge->RunFrame(g_levelTime);
        BuildSnapshot();
        // Trail cg.time by one tick (clamped to snap #1) so priming frames present a valid snap/nextSnap
        // pair, matching SP_DrawFrame. With only one snap so far the clamp pins cg.time to base (nextSnap
        // NULL is fine — nothing flagged interpolate yet).
        int cgTime = g_levelTime - SP_INTERP_DELAY;
        if( cgTime < g_snapBaseTime ) cgTime = g_snapBaseTime;
        g_vmMain(CG_DRAW_ACTIVE_FRAME, cgTime, 0 /*STEREO_CENTER*/, qfalse);
    }
    blog("SP_ClientFrames: drew %d cgame frames to levelTime=%d (numEntities=%d)\n", n, g_levelTime, g_snap.numEntities);
}

extern "C" int SP_IsActive(void){ return g_spActive; }
// True while a save-load/transition is queued (g_transMap set) but the SP cgame hasn't come up yet.
// The load watchdog (sp_integration.c) uses this to retry a starved/dropped efsptrans before it
// gives up and bounces to the main menu.
extern "C" int SP_LoadPending(void){ return g_transMap[0] ? 1 : 0; }
// Bridge-state half of an abort-to-menu (P2-1/P2-2 + memory-audit H2). Called by SP_AbortToMenu when a load
// is abandoned: clear the consume-once pending-load fields so SP_LoadPending() doesn't stay stuck at 1, and
// drop the partial game module (SP_UnloadGame clears g_spActive + zeros the dangling g_linked[]). The engine
// side (clc.state, renderer, SP UI menu) is owned by SP_AbortToMenu. NOTE: if a transition ERR_DROP'd AFTER
// SPR_ShutdownRenderer but before SPR_InitRenderer, the renderer is still down here — that rare mid-transition
// case is handled by SP_AbortToMenu re-bringing the menu up; the common abort (New Game spawn failure) aborts
// before any renderer teardown, so the renderer/menu are live.
extern "C" void SP_AbortState(void){
    g_transMap[0] = 0; g_transSpawn[0] = 0; g_transGameFlag = 0;
    SP_UnloadGame();
}
// The currently-active SP map name (empty if none). Used by SP_MapWrap_f to debounce a spurious
// repeat of 'map <same-map>' that would otherwise reload and hit the renderer's redundant-world guard.
extern "C" const char* SP_CurMap(void){ return g_curMap; }
// Advance the sim by REAL elapsed wall-clock time each rendered frame. This keeps the original's
// smooth per-frame structure (RunFrame every frame -> smooth view) while running at CORRECT speed
// (the prior code added a fixed 50ms/frame -> 3x at 60fps; a 20Hz accumulator was correct-speed but
// choppy/laggy). The delta is clamped so a hitch/pause can't teleport the simulation.
extern "C" void SP_DrawFrame(int serverTime, int stereo){
    (void)serverTime;
    static int fc=0; if(fc<3){ blog("SP_DrawFrame #%d stereo=%d levelTime=%d numEnt=%d\n", fc, stereo, g_levelTime, g_snap.numEntities); } fc++;

    static int lastReal = 0;
    int now = Sys_Milliseconds();
    if(lastReal == 0) lastReal = now;
    int delta = now - lastReal; lastReal = now;
    if(delta < 1) delta = 1; if(delta > 100) delta = 100;   // clamp hitches (no >100ms sim jumps)

    // PAUSE when the in-game menu (ESC) or console is up: retail SP pauses the server; our loopback has no
    // server, so freeze the sim here — do NOT advance g_levelTime or run ClientThink/RunFrame; just re-draw
    // the frozen frame behind the menu. (lastReal is already updated above, so unpausing won't time-jump.)
    { extern int SP_MenuPaused(void);
      static int s_wasPaused = 0;
      if( SP_MenuPaused() ){
        // EF1 SP 1:1: silence in-flight audio ONCE on the unpaused->paused edge. Retail pauses the SERVER,
        // and its UI dll issues "stopsound" (-> S_StopAllSounds) the moment the in-game menu opens (efuix86
        // retail @ ...). The SP loopback has no server pause; the sound mixer clock (s_soundtime from
        // SNDDMA_GetDMAPos) is WALL-CLOCK not sim-time, and CL_Frame calls S_Update every frame, so without
        // this the VO mid-line, explosions, music and ambient loops keep playing audibly while the sim is
        // frozen. Edge-triggered (NOT every frame) so it doesn't re-flush the DMA buffer or kill the menu's
        // own click sounds. Matches retail "pause = stopsound on menu-open"; reset on the unpaused path below.
        if( !s_wasPaused ){ extern void S_StopAllSounds(void); S_StopAllSounds(); s_wasPaused = 1;
            // Grab the save-menu thumbnail ONCE here, on the gameplay->menu edge (not every gameplay
            // second). The capture is a full-screen GPU readback that stalls the Adreno pipeline ~100ms;
            // doing it per-second during play was the dominant stutter. You only save from this menu, so a
            // single capture on menu-open is enough and its one-time hitch is hidden by the transition.
            SPR_RequestSaveThumb(); }
        // Freeze the sim but redraw at the SAME trailing cg.time the active path uses below (g_levelTime and
        // g_simResidual are frozen while paused). Passing g_levelTime here instead would jump cg.time FORWARD
        // to the newest snap during the pause, then BACKWARD on unpause -> CG_ProcessSnapshots ERR_DROP
        // ("cg.snap->serverTime > cg.time") -> bounce to menu (the save-then-exit-menu crash).
        int cgTime = g_levelTime - SP_SV_FRAMEMSEC + g_simResidual;
        if( cgTime >= g_levelTime )    cgTime = g_levelTime - 1;
        if( cgTime < g_snapBaseTime )  cgTime = g_snapBaseTime;
        g_vmMain(CG_DRAW_ACTIVE_FRAME, cgTime, stereo, qfalse);
        // The paused CG_DRAW_ACTIVE_FRAME above re-adds every entity loopSound, and CL_Frame's
        // unconditional per-frame S_Update re-mixes them -> machinery/ambient loops keep humming
        // through the pause (the edge-only S_StopAllSounds can't catch them). Clear the loop list
        // every paused frame so the pause is fully silent. One-shots/VO/music are event-driven and
        // stay dead after the edge stop. (Diverges from retail, which leaves loops faintly audible;
        // intentional — user wants a fully silent pause.)
        { extern void S_ClearLoopingSounds(qboolean); S_ClearLoopingSounds(qtrue); }
        return;
      }
      s_wasPaused = 0;   // unpaused this frame -> re-arm the pause-edge stopsound
    }

    // (Save-menu thumbnail capture is armed on the gameplay->menu edge in the pause block above, not here
    // every second — the per-second full-screen GPU readback was the dominant stutter. SP_SaveGame embeds
    // the latest capture in the .sav 'SHOT' chunk.)

    // Refresh the usercmd from the latest touch/pad input ONCE per render frame (reused by every 50ms tick
    // drained below). serverTime is stamped per-tick inside the loop so pml.msec == 50.
    { int an[3]; signed char fwd=0,side=0,up=0; int btn=0;
      SPR_GetLatestCmd(an,&fwd,&side,&up,&btn);
      g_lastcmd.angles[0]=an[0]; g_lastcmd.angles[1]=an[1]; g_lastcmd.angles[2]=an[2];
      g_lastcmd.forwardmove=fwd; g_lastcmd.rightmove=side; g_lastcmd.upmove=up; g_lastcmd.buttons=btn;
      // Propagate the cgame's selected weapon into the usercmd. The cgame reports its weapon via
      // CG_SETUSERCMDVALUE (-> g_weaponSelect); pmove (ge->ClientThink) switches ps.weapon to cmd.weapon
      // when the player owns it. Without this, cmd.weapon stays 0 and ps.weapon NEVER changes from WP_NONE
      // -> no view weapon EVER, in any mission. This is the general EF weapon-switch path: scripted
      // SET_WEAPON, pickups, weapnext/weapprev all set cg.weaponSelect -> here -> pmove. (Multi-mission.)
      g_lastcmd.weapon = (byte)g_weaponSelect; }

    // FIXED 50ms/20Hz SIM CADENCE — mirror retail SV_Frame (retail:30640,30641,30659-30662):
    // accumulate real elapsed ms, then drain it in fixed SP_SV_FRAMEMSEC quanta, advancing g_levelTime by
    // exactly 50 each tick and emitting ONE snapshot per tick on the 50ms grid.
    g_simResidual += delta;
    if( g_simResidual > SP_SV_FRAMEMSEC * SP_MAX_CATCHUP )   // hitch cap: bound catch-up, no spiral
        g_simResidual = SP_SV_FRAMEMSEC * SP_MAX_CATCHUP;
    while( g_simResidual >= SP_SV_FRAMEMSEC ) {
        g_simResidual -= SP_SV_FRAMEMSEC;
        g_levelTime   += SP_SV_FRAMEMSEC;
        g_cmdNum++;
        g_lastcmd.serverTime = g_levelTime;
        ge->ClientThink(0, &g_lastcmd);
        SP_UpdateVoiceOverride(); ge->RunFrame(g_levelTime);
        BuildSnapshot();
    }

    // Deferred autosave: an autosave requested during a cutscene (in_camera) was held back by SP_SaveGame so it
    // wouldn't capture mid-cinematic state. Write it now that the cutscene has ended, so the death-respawn
    // checkpoint is a clean post-cinematic state instead of one that reloads with the player free-walking the
    // scene. Runs only on active (unpaused) gameplay frames; GameAllowedToSaveHere() == !in_camera.
    if( g_pendingAutosave && ge && g_spActive && (!ge->GameAllowedToSaveHere || ge->GameAllowedToSaveHere()) ){
        blog("save: writing DEFERRED autosave now that the cutscene has ended\n");
        SP_SaveGame("auto");   // clears g_pendingAutosave on the allowed-save path
    }

    // ONE cgame draw per RENDER frame (decoupled from the sim). cg.time advances SMOOTHLY per render frame by
    // adding the real-time residual (the sub-tick fraction toward the next 50ms snap) to the trailing base.
    // So cg.time sweeps (g_levelTime-50 .. g_levelTime) as residual goes 0..49 -> cg.snap (N) and cg.nextSnap
    // (N+1) bracket it and frameInterpolation = residual/50 sweeps (0,1) -> SMOOTH motion. Without the residual
    // cg.time was pinned to the 50ms grid -> frameInterp=0 -> 20Hz stutter. Upper-clamp keeps the newest snap
    // strictly ahead (valid nextSnap); lower-clamp to snap #1 so the first frames never assert.
    int cgTime = g_levelTime - SP_SV_FRAMEMSEC + g_simResidual;
    if( cgTime >= g_levelTime )    cgTime = g_levelTime - 1;
    if( cgTime < g_snapBaseTime )  cgTime = g_snapBaseTime;
    g_vmMain(CG_DRAW_ACTIVE_FRAME, cgTime, stereo, qfalse);
}
