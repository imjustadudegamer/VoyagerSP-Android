// sp_ui_bridge.cpp — bridge for EF1's native SP UI module (libefui.so, GetUIAPI).
// EF1's SP UI is FAKK/Q2 direct-struct style: GetUIAPI() returns a uiexport_t* (12 fns);
// the engine passes a uiimport_t* (59 fns) into UI_Init. NOT the Q3 vmMain/syscall style.
// This bridge fills uiimport_t (renderer/sound via the shared SPR_* path, the rest via the
// lilium engine), loads the module, and exposes neutral SPUI_* drive calls (sp_ui.h) that
// the engine hooks invoke instead of the Holomatch vm/ui.qvm path.
//
// Compiled in the sp_game_bridge static lib (SP UI headers + ef_android_compat.h force-include).
#include "q_shared.h"
#include "tr_types.h"        // refEntity_t, refdef_t, glconfig_t, polyVert_t, orientation_t
// NOTE: do NOT include qcommon.h — it declares the engine funcs (Com_Printf, FS_*, ...) in C++
// linkage, which clashes with the extern "C" block below. We declare them ourselves instead.
#include "ui_public.h"       // uiimport_t / uiexport_t / UI_API_VERSION (src/ui on include path)
#include "sp_render.h"       // SPR_* (shared renderer/sound wrappers)
#include "sp_ui.h"           // neutral SPUI_* interface + HM-side accessors
#include <dlfcn.h>
#include <string.h>
#include <stdarg.h>
#include <android/log.h>

static void ulog(const char* fmt, ...){ va_list a; va_start(a,fmt); __android_log_vprint(ANDROID_LOG_INFO,"ui",fmt,a); va_end(a); }

// Com_Printf/Com_Error/Q_strncpyz are declared C++-linkage by q_shared.h here, so reference
// them via the C-linkage shim (sp_clink.c) / a local copy instead of the mangled symbols.
extern "C" { void (*SP_ComPrintf(void))(const char*,...); void SP_ComError(int,const char*,...); }
static void sp_strncpyz(char* d, const char* s, int n){ if(n<=0)return; int i=0; for(; i<n-1 && s && s[i]; i++) d[i]=s[i]; d[i]=0; }

// ---- lilium engine services (ABI-compatible with the SP boundary types) ----
// Com_Printf / Com_Error are already declared by q_shared.h (C++ linkage) — do not redeclare.
extern "C" {
    int   Com_Milliseconds( void );
    void  Cvar_Set( const char *name, const char *value );
    float Cvar_VariableValue( const char *name );
    void  Cvar_VariableStringBuffer( const char *name, char *buf, int bufsize );
    void  Cvar_SetValue( const char *name, float value );
    void  Cvar_Reset( const char *name );
    void  Cvar_InfoStringBuffer( int bit, char *buf, int bufsize );
    cvar_t *Cvar_Get( const char *name, const char *value, int flags );
    int   Cmd_Argc( void );
    void  Cmd_ArgvBuffer( int arg, char *buffer, int bufferLength );
    void  Cmd_TokenizeString( const char *text );
    void  Cbuf_ExecuteText( int exec_when, const char *text );
    int   FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );
    int   FS_Read( void *buffer, int len, fileHandle_t f );
    int   FS_Write( const void *buffer, int len, fileHandle_t f );
    void  FS_FCloseFile( fileHandle_t f );
    int   FS_GetFileList( const char *path, const char *ext, char *listbuf, int bufsize );
    long  FS_ReadFile( const char *qpath, void **buffer );
    void  FS_FreeFile( void *buffer );
    char *Sys_GetClipboardData( void );
    void  SCR_UpdateScreen( void );
    char *Key_KeynumToString( int keynum );
    char *Key_GetBinding( int keynum );
    void  Key_SetBinding( int keynum, const char *binding );
    qboolean Key_IsDown( int keynum );
    qboolean Key_GetOverstrikeMode( void );
    void  Key_SetOverstrikeMode( qboolean state );
    void  Key_ClearStates( void );
    int   Key_GetCatcher( void );
    void  Key_SetCatcher( int catcher );
}

// ================= uiimport_t adapters =================
// renderer -> shared SPR_* (handles the SP->HM refEntity translation + reType remap)
static void      UII_R_ClearScene(void){ SPR_ClearScene(); }
static void      UII_R_AddRefEntity(const refEntity_t* re){ SPR_AddRefEntity(re, sizeof(*re)); }
static void      UII_R_AddPoly(qhandle_t hShader,int n,const polyVert_t* v){ SPR_AddPoly((int)hShader,n,v); }
static void      UII_R_AddLight(const vec3_t org,float i,float r,float g,float b){ SPR_AddLight(org,i,r,g,b); }
static void      UII_R_RenderScene(const refdef_t* fd){ SPR_RenderScene(fd, sizeof(*fd)); }
static qhandle_t UII_R_RegisterModel(const char* n){ return (qhandle_t)SPR_RegisterModel(n); }
static qhandle_t UII_R_RegisterSkin(const char* n){ return (qhandle_t)SPR_RegisterSkin(n); }
static qhandle_t UII_R_RegisterShader(const char* n){ return (qhandle_t)SPR_RegisterShader(n); }
static qhandle_t UII_R_RegisterShaderNoMip(const char* n){ return (qhandle_t)SPR_RegisterShaderNoMip(n); }
static void      UII_R_SetColor(const float* rgba){ SPR_SetColor(rgba); }
static void      UII_R_DrawStretchPic(float x,float y,float w,float h,float s1,float t1,float s2,float t2,qhandle_t s){ SPR_DrawStretchPic(x,y,w,h,s1,t1,s2,t2,(int)s); }
static void      UII_R_ScissorPic(float x,float y,float w,float h,float s1,float t1,float s2,float t2,qhandle_t s){ SPR_ScissorPic(x,y,w,h,s1,t1,s2,t2,(int)s); }
static void      UII_R_LerpTag(orientation_t* tag,clipHandle_t mod,int sf,int ef,float frac,const char* name){ SPR_LerpTag(tag,(int)mod,sf,ef,frac,name); }
static void      UII_DrawStretchRaw(int x,int y,int w,int h,int cols,int rows,const byte* data,float fLight){ (void)fLight; SPR_DrawStretchRaw(x,y,w,h,cols,rows,data); }

// sound
static void        UII_S_StartLocalSound(sfxHandle_t sfx,int chan){ SPR_S_StartLocalSound((int)sfx,chan); }
static sfxHandle_t UII_S_RegisterSound(const char* name){ return (sfxHandle_t)SPR_S_RegisterSound(name); }
static void        UII_S_StartLocalLoopingSound(sfxHandle_t sfx){ (void)sfx; }   // TODO looping ui sfx

// keys -> Buf adapters over the engine's char*-returning funcs
static void UII_Key_KeynumToStringBuf(int k,char* buf,int len){ sp_strncpyz(buf, Key_KeynumToString(k), len); }
static void UII_Key_GetBindingBuf(int k,char* buf,int len){ const char* b=Key_GetBinding(k); sp_strncpyz(buf, b?b:"", len); }

// cvar/cmd/fs/misc adapters where the engine signature differs slightly
static void UII_Cvar_Create(const char* n,const char* v,int f){ Cvar_Get(n,v,f); }   // engine returns cvar_t*
static int  UII_FS_ReadFile(const char* n,void** buf){ return (int)FS_ReadFile(n,buf); } // long->int
static void UII_GetClipboardData(char* buf,int bufsize){ char* c=Sys_GetClipboardData(); sp_strncpyz(buf, c?c:"", bufsize); }
static void UII_GetGlconfig(glconfig_t* cfg){
    int w=640,h=480; SPR_GetVidSize(&w,&h); if(w<=0)w=640; if(h<=0)h=480;
    memset(cfg,0,sizeof(*cfg));
    cfg->vidWidth=w; cfg->vidHeight=h; cfg->windowAspect=(float)w/(float)h;
}
static connstate_t UII_GetClientState(void){
    // The engine connstate_t has an extra CA_AUTHORIZING at index 2 that the SP UI's enum lacks,
    // so engine values >=2 are one higher (engine CA_ACTIVE=8 vs SP=7). Collapse to the SP enum.
    int e = SPUI_GetClientState();
    return (connstate_t)( (e <= 1) ? e : e - 1 );
}
static void UII_GetConfigString(int index,char* buf,int size){ SPUI_GetConfigString(index,buf,size); }

// savegame/screenshot. The Load/Save menu (ui_game.cpp ReadSaveDirectory) lists files via
// FS_GetFileList, then for EACH calls SG_ValidateForLoadSaveScreen (skips the file if qfalse)
// and SG_GetSaveGameComment (the 128-byte iSG_COMMENT_SIZE comment it displays). These were
// stubbed (return qfalse/0) so the menu showed NOTHING even with valid .sav files present.
// Read the first chunk (COMM, written by SP_SaveGame) from saves/<name>.sav.
// Framing matches sp_bridge.cpp I_Append: [id:4][len:4][cksum:4][data:len][magic:4].
static qboolean SP_UI_ReadSaveComment(const char* name, char out[iSG_COMMENT_SIZE]){
    char path[128]; fileHandle_t f = 0; unsigned id=0, len=0, cks=0;
    if(!name || !*name) return qfalse;
    snprintf(path, sizeof(path), "saves/%s.sav", name);
    FS_FOpenFileByMode(path, &f, FS_READ);
    if(!f) return qfalse;
    qboolean ok = qfalse;
    if(FS_Read(&id,4,f)==4 && FS_Read(&len,4,f)==4 && id=='COMM' && len>=1){
        int n = (int)len; if(n > iSG_COMMENT_SIZE) n = iSG_COMMENT_SIZE;
        FS_Read(&cks,4,f);
        memset(out, 0, iSG_COMMENT_SIZE);
        if(FS_Read(out, n, f)==n) ok = qtrue;
    }
    FS_FCloseFile(f);
    return ok;
}
// Scan saves/<name>.sav for a specific chunk id and copy its data into out (clamped to outMax).
// Framing matches sp_bridge.cpp I_Append: [id:4][len:4][cksum:4][data:len][magic:4]. Non-matching chunks
// are read-and-discarded (no FS_Seek dependency) so this also works for retail-ordered saves; the SHOT
// chunk is the 2nd written (after COMM), so only the tiny COMM precedes it.
static qboolean SP_UI_ReadSaveChunk(const char* name, unsigned wantId, void* out, int outMax){
    char path[128]; fileHandle_t f=0; unsigned id=0, len=0, cks=0, magic=0;
    if(!name || !*name || !out || outMax<=0) return qfalse;
    snprintf(path, sizeof(path), "saves/%s.sav", name);
    FS_FOpenFileByMode(path, &f, FS_READ);
    if(!f) return qfalse;
    qboolean ok = qfalse;
    for(int guard=0; guard<256; guard++){
        if(FS_Read(&id,4,f)!=4 || FS_Read(&len,4,f)!=4) break;
        if(FS_Read(&cks,4,f)!=4) break;
        if(id == wantId){
            int n = (int)len; if(n > outMax) n = outMax;
            if(n>0 && FS_Read(out, n, f)==n) ok = qtrue;
            break;
        }
        // discard this chunk's data + trailing magic to reach the next header
        { char scratch[1024]; unsigned remaining=len;
          while(remaining){ int c=(remaining>sizeof(scratch))?(int)sizeof(scratch):(int)remaining;
              if(FS_Read(scratch,c,f)!=c){ FS_FCloseFile(f); return qfalse; } remaining-=(unsigned)c; } }
        if(FS_Read(&magic,4,f)!=4) break;
    }
    FS_FCloseFile(f);
    return ok;
}
static void     UII_UpdateScreen(void){ SCR_UpdateScreen(); }
static void     UII_PrecacheScreenshot(void){}
// Read the 256x256x4 RGBA SHOT thumbnail from saves/<p>.sav into the UI's screenShotBuf (the menu draws it
// with negative height, flipping the bottom-up capture upright). qfalse -> the UI blacks the preview pane.
static qboolean UII_SG_GetSaveImage(const char* p,void* a){ return SP_UI_ReadSaveChunk(p, 'SHOT', a, 256*256*4); }
static void*    UII_SG_GetSaveGameComment(const char* p){
    static char comment[iSG_COMMENT_SIZE];
    if(SP_UI_ReadSaveComment(p, comment)) return comment;
    return 0;
}
static qboolean UII_SG_ValidateForLoadSaveScreen(const char* p){
    char tmp[iSG_COMMENT_SIZE];
    return SP_UI_ReadSaveComment(p, tmp) ? qtrue : qfalse;
}
static qboolean UII_SG_GameAllowedToSaveHere(qboolean inCamera){ (void)inCamera; return qtrue; }
// Capture the description the Save menu typed; SP_SaveGame (sp_bridge.cpp) reads it for the COMM chunk.
char g_spSaveComment[iSG_COMMENT_SIZE] = {0};
static void     UII_SG_StoreSaveGameComment(const char* s){ sp_strncpyz(g_spSaveComment, s?s:"", iSG_COMMENT_SIZE); }
static byte*    UII_SCR_GetScreenshot(qboolean* b){ if(b)*b=qfalse; return 0; }

// ================= module load / drive =================
typedef uiexport_t* (*GetUIAPI_t)(void);
static void*       g_uilib = 0;
static uiexport_t* g_ux    = 0;
static int         g_uiActive = 0;
static uiimport_t  g_uii;

static void SPUI_FillImport(void){
    memset(&g_uii, 0, sizeof(g_uii));
    g_uii.Printf=SP_ComPrintf(); g_uii.Error=SP_ComError;
    g_uii.Cvar_Set=Cvar_Set; g_uii.Cvar_VariableValue=Cvar_VariableValue;
    g_uii.Cvar_VariableStringBuffer=Cvar_VariableStringBuffer; g_uii.Cvar_SetValue=Cvar_SetValue;
    g_uii.Cvar_Reset=Cvar_Reset; g_uii.Cvar_Create=UII_Cvar_Create; g_uii.Cvar_InfoStringBuffer=Cvar_InfoStringBuffer;
    g_uii.Argc=Cmd_Argc; g_uii.Argv=Cmd_ArgvBuffer; g_uii.Cmd_ExecuteText=Cbuf_ExecuteText; g_uii.Cmd_TokenizeString=Cmd_TokenizeString;
    g_uii.FS_FOpenFile=FS_FOpenFileByMode; g_uii.FS_Read=FS_Read; g_uii.FS_Write=FS_Write; g_uii.FS_FCloseFile=FS_FCloseFile;
    g_uii.FS_GetFileList=FS_GetFileList; g_uii.FS_ReadFile=UII_FS_ReadFile; g_uii.FS_FreeFile=FS_FreeFile;
    g_uii.R_RegisterModel=UII_R_RegisterModel; g_uii.R_RegisterSkin=UII_R_RegisterSkin;
    g_uii.R_RegisterShader=UII_R_RegisterShader; g_uii.R_RegisterShaderNoMip=UII_R_RegisterShaderNoMip;
    g_uii.R_ClearScene=UII_R_ClearScene; g_uii.R_AddRefEntityToScene=UII_R_AddRefEntity;
    g_uii.R_AddPolyToScene=UII_R_AddPoly; g_uii.R_AddLightToScene=UII_R_AddLight; g_uii.R_RenderScene=UII_R_RenderScene;
    g_uii.R_SetColor=UII_R_SetColor; g_uii.R_DrawStretchPic=UII_R_DrawStretchPic; g_uii.R_ScissorPic=UII_R_ScissorPic;
    g_uii.UpdateScreen=UII_UpdateScreen; g_uii.PrecacheScreenshot=UII_PrecacheScreenshot;
    g_uii.R_LerpTag=UII_R_LerpTag;
    g_uii.S_StartLocalSound=UII_S_StartLocalSound; g_uii.S_RegisterSound=UII_S_RegisterSound; g_uii.S_StartLocalLoopingSound=UII_S_StartLocalLoopingSound;
    g_uii.DrawStretchRaw=UII_DrawStretchRaw;
    g_uii.SG_GetSaveImage=UII_SG_GetSaveImage; g_uii.SG_GetSaveGameComment=UII_SG_GetSaveGameComment;
    g_uii.SG_ValidateForLoadSaveScreen=UII_SG_ValidateForLoadSaveScreen; g_uii.SG_GameAllowedToSaveHere=UII_SG_GameAllowedToSaveHere;
    g_uii.SG_StoreSaveGameComment=UII_SG_StoreSaveGameComment; g_uii.SCR_GetScreenshot=UII_SCR_GetScreenshot;
    g_uii.Key_KeynumToStringBuf=UII_Key_KeynumToStringBuf; g_uii.Key_GetBindingBuf=UII_Key_GetBindingBuf;
    g_uii.Key_SetBinding=Key_SetBinding; g_uii.Key_IsDown=Key_IsDown;
    g_uii.Key_GetOverstrikeMode=Key_GetOverstrikeMode; g_uii.Key_SetOverstrikeMode=Key_SetOverstrikeMode;
    g_uii.Key_ClearStates=Key_ClearStates; g_uii.Key_GetCatcher=Key_GetCatcher; g_uii.Key_SetCatcher=Key_SetCatcher;
    g_uii.GetClipboardData=UII_GetClipboardData; g_uii.GetGlconfig=UII_GetGlconfig;
    g_uii.GetClientState=UII_GetClientState; g_uii.GetConfigString=UII_GetConfigString;
    g_uii.Milliseconds=Com_Milliseconds;
}

extern "C" int SPUI_Load(void){
    if(g_uiActive) return 1;
    g_uilib = dlopen("libefui.so", RTLD_NOW|RTLD_GLOBAL);
    if(!g_uilib){ ulog("dlopen libefui.so failed: %s", dlerror()); return 0; }
    GetUIAPI_t getapi = (GetUIAPI_t)dlsym(g_uilib, "GetUIAPI");
    if(!getapi) getapi = (GetUIAPI_t)dlsym(g_uilib, "_Z8GetUIAPIv");   // C++-mangled fallback
    if(!getapi){ ulog("GetUIAPI symbol not found: %s", dlerror()); return 0; }
    g_ux = getapi();
    if(!g_ux || !g_ux->UI_Init){ ulog("GetUIAPI returned null/!UI_Init"); return 0; }
    SPUI_FillImport();
    g_ux->UI_Init(UI_API_VERSION, &g_uii);
    g_uiActive = 1;
    ulog("SP UI loaded (UI_API_VERSION=%d)", UI_API_VERSION);
    return 1;
}
extern "C" void SPUI_Shutdown(void){ if(g_uiActive && g_ux && g_ux->UI_Shutdown) g_ux->UI_Shutdown(); g_uiActive=0; }
extern "C" int  SPUI_IsActive(void){ return g_uiActive; }
extern "C" void SPUI_Refresh(int t){ if(g_uiActive && g_ux->UI_Refresh) g_ux->UI_Refresh(t); }
extern "C" void SPUI_KeyEvent(int key,int down){ if(g_uiActive && down && g_ux->UI_KeyEvent) g_ux->UI_KeyEvent(key); }
extern "C" void SPUI_MouseEvent(int dx,int dy){ if(g_uiActive && g_ux->UI_MouseEvent) g_ux->UI_MouseEvent(dx,dy); }
extern "C" int  SPUI_IsFullscreen(void){
    if(!g_uiActive || !g_ux->UI_GetActiveMenu) return 0;
    // name MUST be NULL: UI_GetActiveMenu does strcpy(*menuname,"unknown") when a menu is up,
    // so a non-NULL char* slot -> strcpy(NULL,...) -> SIGSEGV. The real engine passes NULL here.
    qboolean fs=qfalse; g_ux->UI_GetActiveMenu(NULL,&fs); return fs?1:0;
}
extern "C" void SPUI_SetActiveMenu(const char* name){ if(g_uiActive && g_ux->UI_SetActiveMenu){ ulog("SetActiveMenu(%s)", name?name:"NULL"); g_ux->UI_SetActiveMenu(name, ""); } }
extern "C" int  SPUI_ConsoleCommand(void){ return (g_uiActive && g_ux->UI_ConsoleCommand) ? g_ux->UI_ConsoleCommand() : 0; }
extern "C" void SPUI_DrawConnect(const char* sv,const char* upd){ if(g_uiActive && g_ux->UI_DrawConnect) g_ux->UI_DrawConnect(sv,upd); }
