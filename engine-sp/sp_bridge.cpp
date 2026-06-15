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

// Com_Printf/Com_Error come via the C shim (SP headers declare them C++-linkage).
extern "C" { void (*SP_ComPrintf(void))(const char*,...); void SP_ComError(int,const char*,...); }
static void blog( const char *fmt, ... ){ va_list a; va_start(a,fmt); vfprintf(stdout,fmt,a); va_end(a); fflush(stdout); }

// ---- lilium engine services (resolved at link against the engine objects) ----
extern "C" {
    int   Sys_Milliseconds( void );
    int   Cmd_Argc( void );
    char *Cmd_Argv( int arg );
    void  Cbuf_AddText( const char *text );
    cvar_t *Cvar_Get( const char *name, const char *value, int flags );
    void  Cvar_Set( const char *name, const char *value );
    int   Cvar_VariableIntegerValue( const char *name );
    void  Cvar_VariableStringBuffer( const char *name, char *buf, int bufsize );
    int   FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );
    int   FS_Read( void *buffer, int len, fileHandle_t f );
    int   FS_Write( const void *buffer, int len, fileHandle_t f );
    void  FS_FCloseFile( fileHandle_t f );
    long  FS_ReadFile( const char *qpath, void **buffer );
    void  FS_FreeFile( void *buffer );
    int   FS_GetFileList( const char *path, const char *ext, char *listbuf, int bufsize );
    // CM (collision) — model 0 = world BSP
    void  CM_LoadMap( const char *name, qboolean clientload, int *checksum );
    char *CM_EntityString( void );
    int   CM_NumInlineModels( void );
    int   CM_InlineModel( int index );
    void  CM_ModelBounds( int model, vec3_t mins, vec3_t maxs );
    int   CM_PointContents( const vec3_t p, int model );
    void  CM_BoxTrace( trace_t *results, const vec3_t start, const vec3_t end,
                       const vec3_t mins, const vec3_t maxs, int model, int brushmask, int capsule );
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
static vmMain_t g_vmMain = NULL;                          // cgame entry point
static intptr_t SP_CgameSyscall( intptr_t cmd, ... );    // cgame->engine dispatcher (defined below)

// ---- minimal server world (entities link here so EntitiesInBox/trace can find them) ----
#define SP_MAX_ENT 1024
static gentity_t *g_linked[SP_MAX_ENT];
static int        g_numLinked = 0;

#define CS_MAX 1024
static char g_configstrings[CS_MAX][1024];

static gentity_t *SP_GEntity( int num ){ return (gentity_t*)((unsigned char*)ge->gentities + ge->gentitySize*num); }

// ----- world implementations -----
static void W_LinkEntity( gentity_t *ent ){
    // derive absmin/absmax from currentOrigin + mins/maxs
    for(int i=0;i<3;i++){
        ent->absmin[i] = ent->currentOrigin[i] + ent->mins[i] - 1;
        ent->absmax[i] = ent->currentOrigin[i] + ent->maxs[i] + 1;
    }
    if(!ent->linked){
        ent->linked = qtrue;
        if(g_numLinked < SP_MAX_ENT) g_linked[g_numLinked++] = ent;
    }
}
static void W_UnlinkEntity( gentity_t *ent ){
    if(!ent->linked) return;
    ent->linked = qfalse;
    for(int i=0;i<g_numLinked;i++) if(g_linked[i]==ent){ g_linked[i]=g_linked[--g_numLinked]; break; }
}
static void W_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs,
                     const vec3_t end, int passEntityNum, int contentmask ){
    // World (BSP) collision only for now; entity-vs-entity clipping is a later pass.
    CM_BoxTrace( results, start, end, mins, maxs, 0, contentmask, 0 );
    results->entityNum = (results->fraction != 1.0f) ? 1022 /*ENTITYNUM_WORLD*/ : 1023 /*NONE*/;
    (void)passEntityNum;
}
static int  W_PointContents( const vec3_t p, int passEntityNum ){ (void)passEntityNum; return CM_PointContents(p,0); }
static qboolean W_InPVS( const vec3_t p1, const vec3_t p2 ){
    int l1=CM_PointLeafnum(p1), l2=CM_PointLeafnum(p2);
    unsigned char *vis=CM_ClusterPVS(CM_LeafCluster(l1));
    int c2=CM_LeafCluster(l2);
    if(c2<0) return qfalse;
    return (vis[c2>>3] & (1<<(c2&7))) ? qtrue : qfalse;
}
static void W_SetBrushModel( gentity_t *ent, const char *name ){
    if(!name||name[0]!='*'){ ent->bmodel=qfalse; return; }
    int h = CM_InlineModel( atoi(name+1) );
    CM_ModelBounds( h, ent->mins, ent->maxs );
    ent->bmodel = qtrue;
    ent->contents = -1; // CONTENTS_SOLID-ish; refined later
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
static void  I_SendConsole( const char *t ){ Cbuf_AddText(t); }
static void  I_DropClient( int n, const char *r ){ (void)n;(void)r; }
static void  I_SendServerCmd( int n, const char *fmt, ... ){ (void)n;(void)fmt; }
static void  I_SetCS( int n, const char *s ){ if(n>=0&&n<CS_MAX){ strncpy(g_configstrings[n], s?s:"", 1023); g_configstrings[n][1023]=0; } }
static void  I_GetCS( int n, char *b, int sz ){ if(n>=0&&n<CS_MAX&&sz>0){ strncpy(b,g_configstrings[n],sz-1); b[sz-1]=0; } else if(sz>0) b[0]=0; }
static void  I_GetUserinfo( int n, char *b, int sz ){ (void)n; if(sz>0){ strncpy(b,"\\name\\Munro\\model\\munro",sz-1); b[sz-1]=0; } }
static void  I_SetUserinfo( int n, const char *b ){ (void)n;(void)b; }
static void  I_GetServerinfo( char *b, int sz ){ if(sz>0) b[0]=0; }
static qboolean I_Append( unsigned long c, void *d, int l ){ (void)c;(void)d;(void)l; return qtrue; }
static int   I_ReadSG( unsigned long c, void *d, int l, void **p ){ (void)c;(void)d;(void)l;(void)p; return 0; }
static int   g_sOverride[256];

extern "C" qboolean SP_LoadGame( const char *solib ){
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
    gi.AppendToSaveGame=I_Append; gi.ReadFromSaveGame=I_ReadSG; gi.ReadFromSaveGameOptional=I_ReadSG;
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
    char bsp[256]; snprintf(bsp,sizeof(bsp),"maps/%s.bsp", mapname);
    int checksum=0;
    blog("SP_SpawnServer: CM_LoadMap(%s)\n", bsp);
    CM_LoadMap( bsp, qfalse, &checksum );
    char *entstring = CM_EntityString();
    blog("SP_SpawnServer: %d inline models, entity string %d bytes\n",
               CM_NumInlineModels(), entstring?(int)strlen(entstring):0);
    g_numLinked = 0; g_levelTime = 0;
    if(!ge){ blog("no game loaded\n"); return qfalse; }
    ge->Init( mapname, "", checksum, entstring, g_levelTime, Sys_Milliseconds(), Sys_Milliseconds(),
              (SavedGameJustLoaded_e)0, qfalse );
    blog("SP_SpawnServer: ge->Init done. num_entities=%d linked=%d\n", ge->num_entities, g_numLinked);
    return qtrue;
}

extern "C" void SP_RunFrames( int n ){
    for(int i=0;i<n;i++){ g_levelTime += 100; ge->RunFrame(g_levelTime); }
    blog("SP_RunFrames: ticked %d frames to levelTime=%d, num_entities=%d\n", n, g_levelTime, ge->num_entities);
}

// =====================================================================================
//  CLIENT / CGAME side — loopback driver + cgame->engine syscall dispatcher.
//  Renderer/sound are stubbed in this increment (validates cgame loop + struct reconcile
//  headlessly). Real renderervk/sound get wired in the APK phase.
// =====================================================================================
static glconfig_t g_glconfig;
static gameState_t g_gameState;
static snapshot_t  g_snap;
static int         g_snapNum   = 1;
static int         g_cmdNum    = 1;
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
    gentity_t* p0 = SP_GEntity(0);
    if(p0->client) g_snap.ps = *(playerState_t*)p0->client;
    int n=0;
    for(int i=1; i<ge->num_entities && n<256; i++){
        gentity_t* e = SP_GEntity(i);
        if(!e->inuse) continue;
        g_snap.entities[n++] = e->s;
    }
    g_snap.numEntities = n;
    g_snapNum++;
}

#define A(n) (args[n])
#define P(n) ((void*)args[n])
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
    case CG_ADDCOMMAND:    return 0;
    case CG_SENDCLIENTCOMMAND: return 0;
    case CG_UPDATESCREEN:  return 0;
    // collision -> reuse engine CM (world)
    case CG_CM_LOADMAP:    return 0;   // already loaded server-side
    case CG_CM_NUMINLINEMODELS: return CM_NumInlineModels();
    case CG_CM_INLINEMODEL: return CM_InlineModel((int)A(0));
    case CG_CM_POINTCONTENTS: return CM_PointContents((float*)P(0),(int)A(1));
    case CG_CM_BOXTRACE:   CM_BoxTrace((trace_t*)P(0),(float*)P(1),(float*)P(2),(float*)P(3),(float*)P(4),(int)A(5),(int)A(6),0); return 0;
    // client state (the critical path)
    case CG_GETGLCONFIG:   *(glconfig_t*)P(0) = g_glconfig; return 0;
    case CG_GETGAMESTATE:  *(gameState_t*)P(0) = g_gameState; return 0;
    case CG_GETCURRENTSNAPSHOTNUMBER: *(int*)P(0)=g_snapNum; *(int*)P(1)=g_levelTime; return 0;
    case CG_GETSNAPSHOT:   *(snapshot_t*)P(1) = g_snap; return qtrue;
    case CG_GETSERVERCOMMAND: return qfalse;
    case CG_GETCURRENTCMDNUMBER: return g_cmdNum;
    case CG_GETUSERCMD:    *(usercmd_t*)P(1) = g_lastcmd; return qtrue;
    case CG_SETUSERCMDVALUE: g_weaponSelect=(int)A(0); return 0;
    case CG_MEMORY_REMAINING: return 0x4000000;
    // renderer/sound registration -> return non-zero fake handles so cgame's validity
    // checks pass (no real assets this increment; APK phase wires the real renderer/sound).
    case CG_R_REGISTERMODEL: case CG_R_REGISTERSKIN:
    case CG_R_REGISTERSHADER: case CG_R_REGISTERSHADERNOMIP:
    case CG_S_REGISTERSOUND: { static int h=0; return ++h; }
    // renderer scene / sound playback / ff / ambient -> no-op (no client surface this increment)
    default: return 0;
    }
}

extern "C" qboolean SP_StartClient(void){
    if(!ge||!g_vmMain){ blog("SP_StartClient: no game/cgame\n"); return qfalse; }
    // glconfig (headless defaults)
    memset(&g_glconfig,0,sizeof(g_glconfig));
    g_glconfig.vidWidth=640; g_glconfig.vidHeight=480; g_glconfig.windowAspect=640.0f/480.0f;
    strcpy(g_glconfig.renderer_string,"efsp-headless");
    strcpy(g_glconfig.vendor_string,"efsp");
    strcpy(g_glconfig.version_string,"1.0");
    BuildGameState();
    // connect + spawn the single player
    char* denied = ge->ClientConnect(0, qtrue, (SavedGameJustLoaded_e)0);
    if(denied){ blog("SP_StartClient: ClientConnect denied: %s\n", denied); return qfalse; }
    memset(&g_lastcmd,0,sizeof(g_lastcmd)); g_lastcmd.serverTime=g_levelTime;
    ge->ClientBegin(0, &g_lastcmd, (SavedGameJustLoaded_e)0);
    blog("SP_StartClient: client connected+begun\n");
    ge->RunFrame(g_levelTime);
    BuildSnapshot();
    blog("SP_StartClient: first snapshot numEntities=%d ps.origin=(%.0f %.0f %.0f)\n",
         g_snap.numEntities, g_snap.ps.origin[0], g_snap.ps.origin[1], g_snap.ps.origin[2]);
    blog("SP_StartClient: cgame CG_INIT...\n");
    g_vmMain(CG_INIT, 0, 0, 0);
    blog("SP_StartClient: CG_INIT returned OK\n");
    return qtrue;
}

extern "C" void SP_ClientFrames(int n){
    for(int i=0;i<n;i++){
        g_levelTime += 50;
        g_lastcmd.serverTime = g_levelTime;
        g_cmdNum++;
        ge->ClientThink(0, &g_lastcmd);
        ge->RunFrame(g_levelTime);
        BuildSnapshot();
        g_vmMain(CG_DRAW_ACTIVE_FRAME, g_levelTime, 0 /*STEREO_CENTER*/, qfalse);
    }
    blog("SP_ClientFrames: drew %d cgame frames to levelTime=%d (numEntities=%d)\n", n, g_levelTime, g_snap.numEntities);
}
