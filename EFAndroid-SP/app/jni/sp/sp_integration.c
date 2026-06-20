// sp_integration.c — HM-side glue: the SPR_* renderer wrappers (call the HM `re` refexport),
// the `spmap` console command, and SP-mode state. Compiled with the HM engine headers.
#include "../efcode/qcommon/q_shared.h"
#include "../efcode/qcommon/qcommon.h"
#include "../efcode/renderercommon/tr_types.h"
#include "../efcode/renderercommon/tr_public.h"
#include "sp_render.h"
#include "sp_ui.h"      // SPUI_IsActive/SetActiveMenu + SP_BeginLoad/EndLoad/IsLoading prototypes

#include <signal.h>
#include <unwind.h>
#include <dlfcn.h>
#include <android/log.h>

extern refexport_t re;        // the HM renderer (set up by the HM CL_InitRef)

void SP_DropHM_Activate(void); // Route b: SP path takes ownership of clc.state (defined below, after client.h)
void SP_AbortToMenu(void);     // recover a failed/stuck SP load back to the main menu (defined after client.h)

// ---- crash backtrace logger: the engine never installs a SIGSEGV handler, so native crashes die
// with no info. This logs a symbolized backtrace to logcat (tag crash) then re-raises. ----
struct sp_bt { void **cur; void **end; };
static _Unwind_Reason_Code sp_bt_cb(struct _Unwind_Context *ctx, void *arg){
    struct sp_bt *s = (struct sp_bt *)arg;
    void *pc = (void *)_Unwind_GetIP(ctx);
    if(pc){ if(s->cur == s->end) return _URC_END_OF_STACK; *s->cur++ = pc; }
    return _URC_NO_REASON;
}
// Called from the engine's Sys_SigHandler (sys_main.c) — the handler that actually catches SIGSEGV.
void SP_LogBacktrace(int sig){
    void *buf[40]; struct sp_bt s = { buf, buf+40 };
    _Unwind_Backtrace(sp_bt_cb, &s);
    int n = (int)(s.cur - buf);
    __android_log_print(ANDROID_LOG_ERROR, "crash", "*** SIGNAL %d  (%d frames) ***", sig, n);
    for(int i=0;i<n;i++){
        Dl_info info;
        if(dladdr(buf[i], &info) && info.dli_sname)
            __android_log_print(ANDROID_LOG_ERROR, "crash", "  #%02d  %s + 0x%lx  [%s]", i,
                info.dli_sname, (unsigned long)((char*)buf[i]-(char*)info.dli_saddr),
                info.dli_fname?info.dli_fname:"?");
        else if(dladdr(buf[i], &info) && info.dli_fbase)
            __android_log_print(ANDROID_LOG_ERROR, "crash", "  #%02d  %p  [%s + 0x%lx]", i, buf[i],
                info.dli_fname?info.dli_fname:"?", (unsigned long)((char*)buf[i]-(char*)info.dli_fbase));
        else
            __android_log_print(ANDROID_LOG_ERROR, "crash", "  #%02d  %p  ?", i, buf[i]);
    }
}
void SP_InstallCrashHandler(void){ /* engine's Sys_SigHandler calls SP_LogBacktrace directly */ }

// SP cgame reType (0..7) -> HM ELITEFORCE reType. RT_MODEL(0) maps straight through.
static const int spReTypeToHM[8] = { 0/*MODEL*/, 0/*POLY->MODEL*/, 1/*SPRITE*/, 4/*BEAM*/,
                                     5/*RAIL_CORE*/, 6/*RAIL_RINGS*/, 7/*LIGHTNING*/, 8/*PORTALSURFACE*/ };

void SPR_ClearScene(void){ if(re.ClearScene) re.ClearScene(); }

void SPR_AddRefEntity(const void* spRe, int spSize){
    if(!re.AddRefEntityToScene) return;
    refEntity_t e;                         // HM layout
    int n = spSize < (int)sizeof(e) ? spSize : (int)sizeof(e);
    memset(&e, 0, sizeof(e));
    memcpy(&e, spRe, n);                    // shared prefix (reType..shaderTime, offset 0..131) is layout-identical
    int spReType = (int)e.reType;          // original SP reType (0..7) BEFORE remap
    // SP and HM refEntity_t DIVERGE after shaderTime: SP has vec3 lightDir(132) then float radius(144),
    // rotation(148); HM (ELITEFORCE) has the `data` union right at 132 (sprite.rotation, sprite.radius,
    // vertRGBA). So the raw memcpy drops SP's lightDir into sprite.rotation/radius -> RT_SPRITE radius/rotation
    // become garbage (usually 0 -> zero-size/invisible or mis-rotated). Re-read the SP sprite fields from their
    // true offsets and clear vertRGBA. (SP RT_SPRITE == reType 2; FX disks use AddPolyToScene, not this path.)
    if(spReType == 2 && spSize >= (int)(38 * sizeof(float))){
        const float* spF = (const float*)spRe;
        e.data.sprite.radius   = spF[36];  // SP radius   @ offset 144
        e.data.sprite.rotation = spF[37];  // SP rotation @ offset 148
        memset(e.data.sprite.vertRGBA, 0, sizeof(e.data.sprite.vertRGBA));
    }
    if(spReType >= 0 && spReType < 8) e.reType = (refEntityType_t)spReTypeToHM[spReType];
    re.AddRefEntityToScene(&e);
}
void SPR_AddPoly(int hShader, int numVerts, const void* verts){
    if(re.AddPolyToScene) re.AddPolyToScene((qhandle_t)hShader, numVerts, (const polyVert_t*)verts, 1);
}
void SPR_AddLight(const float* org, float intensity, float r, float g, float b){
    if(re.AddLightToScene) re.AddLightToScene(org, intensity, r, g, b);
}
void SPR_RenderScene(const void* spRd, int spSize){
    if(!re.RenderScene) return;
    refdef_t rd;                            // HM layout (same as SP for the fields used)
    int n = spSize < (int)sizeof(rd) ? spSize : (int)sizeof(rd);
    memset(&rd, 0, sizeof(rd));
    memcpy(&rd, spRd, n);
    re.RenderScene(&rd);
}
void SPR_SetColor(const float* rgba){ if(re.SetColor) re.SetColor(rgba); }
void SPR_DrawStretchPic(float x,float y,float w,float h,float s1,float t1,float s2,float t2,int hShader){
    if(re.DrawStretchPic) re.DrawStretchPic(x,y,w,h,s1,t1,s2,t2,(qhandle_t)hShader);
}
int  SPR_RegisterModel(const char* n){ return re.RegisterModel ? (int)re.RegisterModel(n) : 0; }
int  SPR_RegisterSkin(const char* n){ return re.RegisterSkin ? (int)re.RegisterSkin(n) : 0; }
int  SPR_RegisterShader(const char* n){ return re.RegisterShader ? (int)re.RegisterShader(n) : 0; }
int  SPR_RegisterShaderNoMip(const char* n){ return re.RegisterShaderNoMip ? (int)re.RegisterShaderNoMip(n) : 0; }
// Route b (sp_dropHM): the SP cgame loads the renderer world itself (no HM spmap). Gated so that
// in legacy mode (sp_dropHM 0) the HM cgame's CG_R_LOADWORLDMAP already loaded it -> no double-load.
void SPR_LoadWorld(const char* n){ if(re.LoadWorld && Cvar_VariableIntegerValue("sp_dropHM")) re.LoadWorld(n); }
// Per-map renderer reset before a reload's CG_INIT. Must be CL_InitRenderer (NOT a bare
// re.BeginRegistration): CL_InitRenderer does re.BeginRegistration (-> R_Init, which clears
// tr.worldMapLoaded so the new world loads without the "redundantly load world map" guard, tr_bsp.c)
// AND re-registers the engine-persistent shaders R_Init just invalidated (charSetShader/whiteShader/
// consoleShader + the Android touch-overlay shaders, cl_main.c:3278). The legacy per-map path runs this
// via CL_StartHunkUsers; the Route-b SP path skipped it, so the 2nd world load (Load Game / transition)
// errored AND would have left the HUD/console/touch shaders dangling.
// Per-map renderer reset, SPLIT into a GPU-teardown half and a re-init half so the SP transition can
// run a Hunk_Clear() BETWEEN them. The renderer's image_t structs (and tr.images[] pointers) live in the
// PERMANENT hunk; re.Shutdown(qfalse) walks that list to destroy each VkImage/VkImageView, so it MUST run
// while the hunk is still valid (BEFORE Hunk_Clear). R_Init (CL_InitRenderer) re-allocates into the cleared
// hunk, so it MUST run AFTER. Running the teardown after Hunk_Clear (or after CM_LoadMap reused that memory)
// makes re.Shutdown walk freed/overwritten handles -> vkDestroyImageView SIGSEGV on load (the load-game crash).
//
// SPR_ShutdownRenderer: re.Shutdown(qfalse) == RE_Shutdown(REF_KEEP_CONTEXT==0) — R_DeleteTextures +
// vk_release_resources (-> qvkResetDescriptorPool) while KEEPING the Vulkan context/device. Resetting the
// descriptor pool here is also what prevents qvkAllocateDescriptorSets VK_ERROR_OUT_OF_POOL_MEMORY after
// ~7 loads. Reload-only path (the fresh New-Game path never calls this).
void SPR_ShutdownRenderer(void){
    if( re.Shutdown ) re.Shutdown( qfalse );
}
// SPR_InitRenderer: CL_InitRenderer -> re.BeginRegistration (R_Init clears tr.worldMapLoaded so the new
// world loads) AND re-registers the engine-persistent shaders R_Init invalidated (charSet/white/console +
// the Android touch-overlay shaders, cl_main.c:3278). Must be CL_InitRenderer, NOT a bare re.BeginRegistration.
void SPR_InitRenderer(void){
    extern void CL_InitRenderer(void);
    CL_InitRenderer();
}
// Full back-to-back reset for callers that do NOT interleave a Hunk_Clear (kept for compatibility).
void SPR_BeginRegistration(void){
    SPR_ShutdownRenderer();
    SPR_InitRenderer();
}
int  SP_DropHM(void){ return Cvar_VariableIntegerValue("sp_dropHM"); }
void SPR_ModelBounds(int model, float* mins, float* maxs){ if(re.ModelBounds) re.ModelBounds((qhandle_t)model, mins, maxs); }

// CG_R_LERPTAG (#53): the SP cgame positions tag-attached models (notably the view weapon,
// the one persistent entity per scene) by calling this into an UNINITIALIZED orientation_t.
// The bridge previously fell through to default:return 0, leaving stale NaN stack bytes that
// became the entity origin -> the "refEntity NaN origin" warning. re.LerpTag always WRITES the
// struct (zeros + identity axis when the tag is missing), eliminating the uninitialized read.
int SPR_LerpTag(void* tag, int model, int startFrame, int endFrame, float frac, const char* tagName){
    if(!re.LerpTag){ memset(tag, 0, sizeof(orientation_t)); return 0; }
    return re.LerpTag((orientation_t*)tag, (qhandle_t)model, startFrame, endFrame, frac, tagName);
}

// ---- SP map-load screen: suppress the parallel HM "SOLO HOLOMATCH" loading splash and
// show the real per-map levelshot, like retail EF1 SP. The whole load (spmap -> efsp) runs
// with the SP cgame torn down / not yet inited, so the HM cgame+ui would otherwise own the
// CA_CONNECTING..CA_ACTIVE draw window. SP_IsLoading() gates SCR_DrawScreenField (cl_scrn.c)
// to draw SP_DrawLoadingScreen() instead, for cold-boot New Game, maptransition,
// loadtransition and load (all four route through `spmap <map>`). ----
static int       g_spLoading = 0;
static char      g_spLoadMap[64] = {0};

void SP_BeginLoad(const char* map){
    g_spLoading = 1;
    Q_strncpyz(g_spLoadMap, map ? map : "", sizeof(g_spLoadMap));
}
void SP_EndLoad(void){ g_spLoading = 0; }
int  SP_IsLoading(void){ return g_spLoading; }
// SP_DrawLoadingScreen() is defined lower down, after client.h is included (it needs `cls`).

// ---- spmap console command: load the SP game + spawn a map ----
extern qboolean SP_LoadGame(const char* solib);
extern qboolean SP_SpawnServer(const char* mapname);
extern qboolean SP_StartClient(void);

static void SP_Map_f(void){
    const char* map = (Cmd_Argc() > 1) ? Cmd_Argv(1) : "borg1";
    SP_BeginLoad(map);   // hold the SP loading screen through efsp -> SP_StartClient (covers direct `efsp`)
    Com_Printf("spmap: loading SP game module + map '%s'\n", map);
    // Recovery on every failure path: otherwise SP_BeginLoad left g_spLoading=1 and the engine sits on the
    // levelshot forever (the watchdog can't see a stuck loading screen). SP_AbortToMenu clears it + menu.
    if(!SP_LoadGame("libefgame.so")){ Com_Printf("spmap: failed to load libefgame.so\n"); SP_AbortToMenu(); return; }
    if(!SP_SpawnServer(map)){ Com_Printf("spmap: SpawnServer failed\n"); SP_AbortToMenu(); return; }
    if(!SP_StartClient()){ Com_Printf("spmap: StartClient failed\n"); SP_AbortToMenu(); return; }
    if( Cvar_VariableIntegerValue("sp_dropHM") ) SP_DropHM_Activate();   // no HM connect drove clc.state; SP owns it
    Com_Printf("spmap: SP map '%s' loaded — switching to SP render mode\n", map);
}

// The SP UI's "New Game" emits a bare `map <first>` (ui_game.cpp ID_STARTNEWGAME ->
// Cbuf "map borg1"). Route it through the proven SP load sequence: spmap (HM spawns the world/BSP
// for the renderer+CM) -> wait for it -> efsp (loads libefgame.so + the SP game via the bridge).
// This mirrors the working autoexec "spmap borg1; wait; efsp borg1".
extern void Cmd_RemoveCommand( const char *cmd_name );
extern void Cbuf_AddText( const char *text );

static void SP_MapWrap_f(void){
    const char* map = (Cmd_Argc() > 1) ? Cmd_Argv(1) : "borg1";
    char buf[256];
    SP_BeginLoad(map);   // show the SP levelshot for the whole spmap->efsp window (kills the HM splash)
    Com_Printf("SP map '%s': spawning world + SP game\n", map);
    if( Cvar_VariableIntegerValue("sp_dropHM") )
        Com_sprintf(buf, sizeof(buf), "efsp %s\n", map);                       // Route b: SP loads its own world; no HM server/cgame
    else
        Com_sprintf(buf, sizeof(buf), "spmap %s\nwait 200\nefsp %s\n", map, map);  // legacy: HM spmap loads world + drives clc.state
    Cbuf_AddText(buf);
}

// Phase 3 transitions + interaction. G_ChangeMap emits maptransition/loadtransition; `use` drives
// the turbolift/triggers. These reach the bridge (sp_bridge.cpp).
extern void SP_MapTransition( const char *map, const char *spawn, int isHub );
extern qboolean SP_FinishTransition( void );   // qtrue iff the SP cgame is fully up (g_spActive=1)
extern void SP_ClientCommand( void );
static void SP_MapTransition_f(void){ SP_MapTransition( Cmd_Argv(1), Cmd_Argv(2), 0 ); }
static void SP_LoadTransition_f(void){ SP_MapTransition( Cmd_Argv(1), Cmd_Argv(2), 1 ); }
// Finish a deferred reload. CRITICAL: only mark the client CA_ACTIVE if the SP cgame actually came up.
// Previously SP_DropHM_Activate ran unconditionally, so a bailed/partial SP_FinishTransition left the
// engine in CA_ACTIVE with g_spActive==0 -> nothing draws the world -> the intermittent "black screen +
// touch overlay only" on load. On failure, recover to the main menu instead of stranding the user.
static void SP_FinishTrans_f(void){
    // EF1 SP: `efsptrans` can be queued REDUNDANTLY in one Cbuf drain. Notably the brig: friendly-fire
    // retaliation arms loadBrigTimer -> G_ChangeMap("_brig") re-emits `maptransition _brig` every frame
    // while ffireLevel>=RETALIATION, so two `maptransition`->`efsptrans` land back-to-back. The 1st
    // SP_FinishTransition loads _brig and CONSUMES g_transMap (clears it); the 2nd then sees g_transMap
    // empty. SP_FinishTransition is stateful (consume-once), unlike retail's idempotent re-spawn, so the
    // 2nd call returned qfalse and we wrongly aborted to the main menu MID-MISSION. Guard: if there's no
    // pending map AND the SP cgame is already live, this efsptrans is a redundant post-success duplicate ->
    // harmless no-op. A genuinely starved/failed load keeps g_transMap set (LoadPending) or g_spActive==0,
    // so it still falls through to the abort/recovery + watchdog path exactly as before.
    { extern int SP_LoadPending(void); extern int SP_IsActive(void);
      if( !SP_LoadPending() && SP_IsActive() ){
          Com_Printf("EFSP: redundant efsptrans (transition already completed) -> no-op\n");
          return;
      } }
    if( SP_FinishTransition() ){
        if( Cvar_VariableIntegerValue("sp_dropHM") ) SP_DropHM_Activate();
    } else {
        Com_Printf("EFSP: SP_FinishTransition did not complete -> not activating; returning to menu\n");
        SP_AbortToMenu();
    }
}
// "use <targetname>" is a SERVER console command -> the game's exported ConsoleCommand
// (g_svcmds.cpp: ConsoleCommand -> Svcmd_Use_f -> G_UseTargets2). It is NOT a ClientCommand verb
// (g_cmds.cpp ClientCommand has no "use"), so routing it to SP_ClientCommand silently did nothing
// -> the Virtual Voyager turbolift menu's "use tour_turbo_NN" never changed decks, and no scripted
// "use <ent>" worked. Route it to the game's ConsoleCommand instead.
static void SP_Use_f(void){ extern int SP_GameConsoleCommand(void); SP_GameConsoleCommand(); }
extern void SP_SaveGame( const char *name );
extern void SP_LoadGameFile( const char *name );
static void SP_Save_f(void){
    // Retail's `save` strips the trailing '*' autosave/thumbnail marker (g_target.cpp emits "save auto*").
    // Without this the autosave lands as saves/auto*.sav -> the Load menu's AUTOSAVE button (`load auto`)
    // can't find it and the file leaks into the save list mislabeled "auto*".
    char nm[64];
    Q_strncpyz( nm, Cmd_Argc()>1 ? Cmd_Argv(1) : "quik", sizeof(nm) );
    { int l = (int)strlen(nm); if( l>0 && nm[l-1]=='*' ) nm[l-1] = '\0'; }
    SP_SaveGame( nm );
}
static void SP_Load_f(void){
    const char* arg = Cmd_Argc()>1 ? Cmd_Argv(1) : "quik";
    // EF1 SP 1:1: the death screen's "press attack to reload" calls respawn() (g_client.cpp), which for a
    // normal (non-holodeck) level sends `load *respawn`. Retail's `*respawn` reloads the `auto` autosave
    // (there is NO respawn.sav; the leading `*` is a special load sentinel, NOT a strippable suffix). The
    // port previously opened saves/*respawn.sav -> not found -> silent no-op, so pressing attack did nothing.
    if( !Q_stricmp( arg, "*respawn" ) ) arg = "auto";
    else if( arg[0] == '*' )           arg = arg + 1;   // strip the sentinel for any other *<name> load
    SP_LoadGameFile( arg );
}
// The UI's "delete save" button (ui_game.cpp ID_DELETEGAMEDATA) issues "wipe <name>". The port never
// registered this command, so deleting a save from the Load/Save menu did nothing. Retail (stvoy) deletes
// saves/<name>.sav and refuses 'auto'/'exitholodeck'. Saves live under fs_homepath -> FS_HomeRemove.
extern void FS_HomeRemove( const char *homePath );
static void SP_Wipe_f(void){
    const char* name = Cmd_Argc()>1 ? Cmd_Argv(1) : "";
    if(!name[0]){ Com_Printf("USAGE: wipe <name>\n"); return; }
    if(!Q_stricmp(name,"auto") || !Q_stricmp(name,"exitholodeck")){
        Com_Printf("^1Can't wipe 'auto' or 'exitholodeck'\n"); return;
    }
    char path[96]; Com_sprintf(path,sizeof(path),"saves/%s.sav", name);
    FS_HomeRemove(path);
    Com_Printf("wiped %s\n", path);
}

void SP_RegisterCommands(void){
    SP_InstallCrashHandler();              // symbolized backtraces for native crashes
    Cmd_AddCommand("efsp", SP_Map_f);
    Cmd_RemoveCommand("map");              // SP app: route the UI's New-Game "map <x>" to the SP loader
    Cmd_AddCommand("map", SP_MapWrap_f);
    Cmd_AddCommand("maptransition",  SP_MapTransition_f);   // forward level change (borg1->borg2)
    Cmd_AddCommand("loadtransition", SP_LoadTransition_f);  // hub/turbolift change
    Cmd_AddCommand("efsptrans",      SP_FinishTrans_f);     // internal: finish a transition reload
    Cmd_AddCommand("use",            SP_Use_f);             // turbolift / trigger interaction
    Cmd_RemoveCommand("save"); Cmd_AddCommand("save", SP_Save_f);   // user save (retail-byte-compatible)
    Cmd_RemoveCommand("load"); Cmd_AddCommand("load", SP_Load_f);   // user load
    Cmd_RemoveCommand("wipe"); Cmd_AddCommand("wipe", SP_Wipe_f);   // delete a save (UI delete button -> "wipe <name>")
    // Route b toggle: 1 = SP cgame is the SOLE cgame (no HM spmap/qagame/cgame) — the proven,
    // intended SP path; 0 = legacy spmap path (HM qagame/cgame run in parallel), kept only as a
    // fallback. Now DEFAULTS to 1 so SP-only is the boot behavior without an on-device autoexec.
    Cvar_Get("sp_dropHM", "1", CVAR_ARCHIVE);
    Com_Printf("SP: map/transition/use/save/load commands registered\n");
}

// Real surface size from the engine's renderer (so the SP cgame builds a full-screen view,
// not the placeholder 640x480 that renders into the corner).
#include "../efcode/client/client.h"
void SPR_GetVidSize(int* w, int* h){ *w = cls.glconfig.vidWidth; *h = cls.glconfig.vidHeight; }

// SP map-load screen draw (needs cls from client.h). Full-screen per-map levelshot; the
// idTech3 image loader resolves the ".tga" request to the shipped ".jpg". Cached per-map so
// the NoMip upload happens once (mid-2D-pass re-registration can corrupt the texture).
void SP_DrawLoadingScreen(void){
    extern const char* SP_LoadingMapName(void);
    const char* map = SP_LoadingMapName();
    int w = cls.glconfig.vidWidth, h = cls.glconfig.vidHeight;
    qhandle_t shot = 0;
    // Register fresh EVERY frame (do NOT cache a handle): CL_FlushMemory runs mid-load and
    // invalidates renderer handles, so a latched handle goes stale -> black. renderervk uploads
    // via a decoupled staging command buffer, so registering during the 2D pass is safe and the
    // lookup is cached/cheap once the image is reachable. The ".tga" request falls through to the
    // shipped ".jpg" in R_LoadImage.
    if( map[0] ) {
        char name[128];
        Com_sprintf(name, sizeof(name), "levelshots/%s.tga", map);
        shot = re.RegisterShaderNoMip(name);
    }
    re.SetColor( NULL );
    if( shot ) {
        re.DrawStretchPic( 0, 0, w, h, 0, 0, 1, 1, shot );
        // EF1 SP 1:1: retail CG_DrawInformation (efgame cg_info.cpp) overlays the `levelShotDetail`
        // scanline texture on top of the levelshot. Draw it here too (no-op if the shader isn't in the
        // user's pak0 -> handle 0). The animated LCARS load bar + quoted CS_MESSAGE level name are
        // cgame-drawn and CANNOT be reproduced in this Route-b load window (the SP cgame is torn down
        // during the load, CS_MESSAGE isn't parsed until ge->Init) -> those need the cgame-alive load
        // restructure (deferred, device-verified).
        qhandle_t detail = re.RegisterShaderNoMip( "levelShotDetail" );
        if( detail ) re.DrawStretchPic( 0, 0, w, h, 0, 0, 2.5f, 2.0f, detail );
    } else {
        float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };   // no/not-yet-loadable art -> black, never the HM splash
        re.SetColor( black );
        re.DrawStretchPic( 0, 0, w, h, 0, 0, 0, 0, cls.whiteShader );
        re.SetColor( NULL );
    }
}
const char* SP_LoadingMapName(void){ return g_spLoadMap; }

// Route b: with no HM spmap/connect, nothing drove clc.state to CA_ACTIVE and nothing called
// re.EndRegistration (CL_InitCGame does both in legacy mode). The SP load path calls this once its
// EF1 SP (Route-b): re-initialize the SP UI if the engine's load flow tore it down. The load path
// (CL_FlushMemory -> CL_ClearMemory -> CL_ShutdownAll -> CL_ShutdownUI -> SPUI_Shutdown) sets g_uiActive=0;
// the stock/HM path re-loads it via CL_StartHunkUsers -> CL_InitUI, but Route-b drives the renderer itself
// (SPR_InitRenderer) and SKIPS CL_StartHunkUsers, so nothing brings the UI back. With g_uiActive=0 (and
// uivm==NULL in Route-b) SCR_UpdateScreen's render gate (uivm || SPUI_IsActive() || com_dedicated) is FALSE
// -> SCR_DrawScreenField never runs -> the freshly loaded game never draws (the load-game HANG). Owning the
// re-init here (rather than inline) keeps the Route-b UI lifecycle in one named place. No-op if UI is up.
void SP_ReinitUI(void){
    if( !SPUI_IsActive() ){
        extern void CL_InitUI( void );
        cls.uiStarted = qtrue;          // mirror CL_StartHunkUsers' bookkeeping
        CL_InitUI();
        Com_Printf("EFSP Route-b: re-initialized SP UI after load (SPUI_IsActive=%d)\n", SPUI_IsActive());
    }
}

// cgame is up: mark the client active so SCR_DrawScreenField -> CL_CGameRendering -> SP_DrawFrame runs,
// and finalize media (frees registration scratch + prewarms Vulkan pipelines).
void SP_DropHM_Activate(void){
    Com_Printf("EFSP Route-b: HM spmap/qagame/cgame NOT loaded; SP cgame is sole; clc.state=CA_ACTIVE\n");
    clc.state = CA_ACTIVE;
    SP_ReinitUI();   // restore the SP UI the engine's load flow tore down (else the render gate freezes — see above)
    // Dismiss the menu so the loaded game is actually visible. The Route-b bridge bypasses CL_MapLoading,
    // which in the stock/HM path clears the catcher on every map load (cl_main.c CL_MapLoading -> Key_SetCatcher(0)).
    // Without this, KEYCATCH_UI stays set and the fullscreen SP main menu keeps drawing OVER the running
    // game -> the user thinks New Game "bounced to the menu" and re-clicks, triggering a redundant 2nd load.
    Key_SetCatcher( 0 );
    if( SPUI_IsActive() ) SPUI_SetActiveMenu( NULL );
    if( re.EndRegistration ) re.EndRegistration();
    // Input P1-1: clear the sticky crouch-toggle on every load/transition activation. tc_crouchToggled is a
    // libmain static that the engine's key-clear doesn't touch, so a level entered while crouch-toggled would
    // invert the first CROUCH tap on the new map. (Touch fingers are released by IN_TouchReset on finger-up;
    // this covers the cross-transition carry the audit flagged.)
    { extern void IN_ClearCrouchToggle(void); IN_ClearCrouchToggle(); }
}

// ---- load-failure recovery + watchdog (fixes the intermittent "load black screen") ----
// SP_AbortToMenu: cleanly leave a failed/stuck SP load. Stop the load screen, drop the engine out of
// CA_ACTIVE (so the world-draw path is no longer expected) and bring the SP main menu back up with input
// focus. Without this, a load that doesn't complete leaves clc.state==CA_ACTIVE while g_spActive==0 ->
// SP_DrawFrame is gated off -> nothing draws the world and the user is stuck on a black screen with only
// the touch overlay (intermittent load black screen).
void SP_AbortToMenu(void){
    { extern void S_StopAllSounds(void); S_StopAllSounds(); }   // else the in-game weapon/VO/ambient loops
                                                                // keep playing into the menu (the phaser leak).
                                                                // The normal ESC->menu and CA_DISCONNECTED
                                                                // bringup paths S_StopAllSounds; this direct
                                                                // abort bypassed them.
    SP_EndLoad();                       // stop the SP per-map load screen if it was up
    { extern void SP_AbortState(void); SP_AbortState(); }   // clear pending-load state + drop the partial game
                                                            // module (P2-1/P2-2/H2): else SP_LoadPending() stays
                                                            // stuck at 1 and the half-spawned module survives.
    clc.state = CA_DISCONNECTED;        // SCR_DrawScreenField(case CA_DISCONNECTED) -> SP main menu
    Key_SetCatcher( KEYCATCH_UI );      // menu takes touch/keys
    if( SPUI_IsActive() ) SPUI_SetActiveMenu( "main" );
    Com_Printf("EFSP: returned to SP main menu (load aborted/stuck)\n");
}

// The SP loopback has no server to pause, so the bridge must pause its own sim when a menu/console is up.
// Returns 1 when the in-game menu (ESC) or console is open -> SP_DrawFrame freezes time + skips RunFrame.
// EF1 SP 1:1: also pause on cl_paused. Retail's in-game cinematic sets cl_paused=1
// to freeze the live game while the .bik overlays it, then SCR_StopCinematic clears it to resume the
// mission. Mirroring retail SV_Frame (which skips the sim while sv_paused/cl_paused) keeps the world
// frozen behind the FMV; the cgame still re-draws that frozen frame so the overlay sits on top.
int SP_MenuPaused(void){
    if ( Key_GetCatcher() & ( KEYCATCH_UI | KEYCATCH_CONSOLE ) ) return 1;
    if ( Cvar_VariableIntegerValue( "cl_paused" ) ) return 1;
    return 0;
}

// SP_LoadWatchdog: called once per rendered frame (SCR_DrawScreenField). Detects the stuck state where the
// engine is CA_ACTIVE but the SP cgame never came up (g_spActive==0) and we are not on the load screen.
// In the happy path this lasts 0 frames (efsptrans sets g_spActive=1 and CA_ACTIVE together, before any
// draw), so the counter never advances and the watchdog is invisible. It only fires on a genuine stall:
//   - frame 30: if a load is still pending, the deferred `efsptrans` was likely Cbuf-starved -> retry it once.
//   - frame 90: still not up -> recover to the main menu rather than sit on a dead black screen.
void SP_LoadWatchdog(void){
    extern int SP_IsActive(void);
    extern int SP_IsLoading(void);
    extern int SP_LoadPending(void);
    static int stuck = 0;
    // Stuck LOADING SCREEN: SP_BeginLoad set g_spLoading=1 but a failure path never reached SP_EndLoad
    // (early return / ERR_DROP), so SCR_DrawScreenField draws the levelshot forever. The main stuck-check
    // below excludes SP_IsLoading(), so cover it here: if the load screen has been up far longer than any
    // real load, recover to the menu. (~10s at 60fps; deck02, the slowest, loaded in ~2s.)
    { static int loadStuck = 0;
      if( SP_IsLoading() ){
          if( ++loadStuck > 600 ){
              Com_Printf("SP-watchdog: loading screen stuck >%d frames -> recovering to main menu\n", loadStuck);
              SP_AbortToMenu(); loadStuck = 0;
          }
      } else loadStuck = 0;
    }
    if( !Cvar_VariableIntegerValue("sp_dropHM") ){ stuck = 0; return; }   // legacy path manages its own state
    if( clc.state == CA_ACTIVE && !SP_IsActive() && !SP_IsLoading() ){
        stuck++;
        if( stuck == 1 )
            Com_Printf("SP-watchdog: CA_ACTIVE but SP cgame not up (g_spActive=0, loadPending=%d) — watching\n", SP_LoadPending());
        if( stuck == 30 && SP_LoadPending() ){
            Com_Printf("SP-watchdog: SP load pending after 30 frames -> retrying efsptrans (was likely Cbuf-starved)\n");
            Cbuf_AddText("efsptrans\n");     // deterministic single retry; SP_FinishTrans_f self-recovers on failure
        }
        if( stuck > 90 ){
            Com_Printf("SP-watchdog: SP load still not up after %d frames -> recovering to main menu\n", stuck);
            SP_AbortToMenu();
            stuck = 0;
        }
    } else {
        stuck = 0;
    }
}

// ---- input: hand the SP loop the engine's current usercmd (reuses HM touch/keys) ----
void SPR_GetLatestCmd(int* angles, signed char* fwd, signed char* side, signed char* up, int* buttons){
    usercmd_t* c = &cl.cmds[ cl.cmdNumber & 63 ];   // CMD_BACKUP-1
    angles[0]=c->angles[0]; angles[1]=c->angles[1]; angles[2]=c->angles[2];
    *fwd=c->forwardmove; *side=c->rightmove; *up=c->upmove;
    *buttons=(int)c->buttons;
}

// ---- sound: route the SP cgame's CG_S_* to the engine's sound system ----
#include "../efcode/client/snd_public.h"
int  SPR_S_RegisterSound(const char* name){ return (int)S_RegisterSound(name, qfalse); }
void SPR_S_StartSound(const float* origin, int entnum, int entchannel, int sfx){ S_StartSound((vec_t*)origin, entnum, entchannel, (sfxHandle_t)sfx); }
void SPR_S_StartSoundVol(const float* origin, int entnum, int entchannel, int sfx, int volume){ S_StartSoundVolume((vec_t*)origin, entnum, entchannel, (sfxHandle_t)sfx, volume); }
void SPR_S_StartLocalSound(int sfx, int channel){ S_StartLocalSound((sfxHandle_t)sfx, channel); }
void SPR_S_ClearLoopingSounds(void){ S_ClearLoopingSounds(qtrue); }
void SPR_S_AddLoopingSound(int entnum, const float* origin, const float* velocity, int sfx){ S_AddLoopingSound(entnum, origin, velocity, (sfxHandle_t)sfx); }
void SPR_S_UpdateEntityPosition(int entnum, const float* origin){ S_UpdateEntityPosition(entnum, origin); }
void SPR_S_Respatialize(int entnum, const float* origin, float (*axis)[3], int inwater){ S_Respatialize(entnum, origin, axis, inwater); }
void SPR_S_StartBackgroundTrack(const char* intro, const char* loop){ S_StartBackgroundTrack(intro, loop); }
int  SPR_S_GetSampleLength(int sfx){ int d = S_SoundDuration((sfxHandle_t)sfx); return d>0 ? d : 2500; }  // ms; fallback if backend lacks it

// ---- SP UI (libefui.so) renderer + state accessors ----
void SPR_DrawStretchRaw(int x,int y,int w,int h,int cols,int rows,const unsigned char* data){
    if(re.DrawStretchRaw) re.DrawStretchRaw(x,y,w,h,cols,rows,(const byte*)data,0,qtrue);
}
// EFSP savegame thumbnail: arm/fetch the renderer's framebuffer capture (256x256x4 RGBA, bottom-up).
void SPR_RequestSaveThumb(void){ if(re.RequestSaveThumb) re.RequestSaveThumb(); }
int  SPR_GetSaveThumb(unsigned char* dst){ return (re.GetSaveThumb && re.GetSaveThumb((byte*)dst)) ? 1 : 0; }
void SPR_ScissorPic(float x,float y,float w,float h,float s1,float t1,float s2,float t2,int hShader){
    // HM renderer has no ScissorPic; draw unclipped (UI clipping is cosmetic for menus).
    if(re.DrawStretchPic) re.DrawStretchPic(x,y,w,h,s1,t1,s2,t2,(qhandle_t)hShader);
}
// CG_R_GETLIGHTING (#45): the cgame samples world lighting at a point — drives AI stealth (enemy
// detection by how lit the player is) + dynamic model lighting. Without it the cgame read garbage.
void SPR_GetLighting(const float* origin, float* ambient, float* directed, float* lightDir){
    if(re.LightForPoint) re.LightForPoint((float*)origin, ambient, directed, lightDir);
    else { ambient[0]=ambient[1]=ambient[2]=64; directed[0]=directed[1]=directed[2]=0;
           lightDir[0]=lightDir[1]=0; lightDir[2]=1; }
}
// CG_CM_MARKFRAGMENTS (#25): bullet/scorch/blood decal projection onto world geometry.
int SPR_MarkFragments(int numPoints, const void* points, const float* projection, int maxPoints,
                      float* pointBuffer, int maxFragments, void* fragmentBuffer){
    if(re.MarkFragments) return re.MarkFragments(numPoints, (const vec3_t*)points, projection,
                                  maxPoints, pointBuffer, maxFragments, (markFragment_t*)fragmentBuffer);
    return 0;
}
int  SPUI_GetClientState(void){ return (int)clc.state; }
void SPUI_GetConfigString(int index, char* buf, int size){ if(buf && size>0) buf[0]=0; }  // stub: menus don't need configstrings
