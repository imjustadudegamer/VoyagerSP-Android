// host_sp_test.cpp — M1.0 on-device ABI bridge proof.
//
// Standalone harness (no engine): dlopen libefgame.so, resolve GetGameAPI/dllEntry/vmMain,
// fill a minimal game_import_t with stub services, call GetGameAPI, validate the returned
// game_export_t (apiversion, gentitySize), then attempt ge->Init() with an empty world to
// see how far the game module's init runs against stub services. Proves the dlopen+ABI path
// works on real Android hardware before wiring the full engine.
//
// Build: NDK clang++ armv7; run: adb shell from /data/local/tmp with LD_LIBRARY_PATH set.

#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>

// SP game ABI + types (same headers the module was compiled against)
#include "q_shared.h"
#include "g_public.h"

static FILE* L;
#define LOG(...) do { fprintf(L?L:stdout, __VA_ARGS__); fflush(L?L:stdout); } while(0)

// ---- stub engine services (just enough for GetGameAPI + a peek at Init) ----
static cvar_t  g_dummyCvar;                 // returned for every cvar() call
static int     g_sOverride[256];            // S_Override target

static void  S_Printf(const char* fmt, ...){ va_list a; va_start(a,fmt); vprintf(fmt,a); va_end(a); fflush(stdout); }
static void  S_WriteCam(const char*){}
static void  S_Error(int, const char* fmt, ...){ va_list a; va_start(a,fmt); LOG("[game ERROR] "); vfprintf(L?L:stdout,fmt,a); va_end(a); LOG("\n"); }
static int   S_Milliseconds(){ return 0; }
static cvar_t* S_cvar(const char* n, const char* v, int){ memset(&g_dummyCvar,0,sizeof(g_dummyCvar)); g_dummyCvar.string=(char*)(v?v:""); return &g_dummyCvar; }
static void  S_cvar_set(const char*, const char*){}
static int   S_CvarInt(const char*){ return 0; }
static void  S_CvarStr(const char*, char* buf, int n){ if(n) buf[0]=0; }
static int   S_argc(){ return 0; }
static char* S_argv(int){ static char e[1]={0}; return e; }
static int   S_FS_FOpenFile(const char*, fileHandle_t* f, fsMode_t){ if(f)*f=0; return -1; }
static int   S_FS_Read(void*, int, fileHandle_t){ return 0; }
static int   S_FS_Write(const void*, int, fileHandle_t){ return 0; }
static void  S_FS_FCloseFile(fileHandle_t){}
static const char* g_base = "./base";       // real game files served from here
static int   S_FS_ReadFile(const char* n, void** buf){
    char p[1024]; snprintf(p,sizeof(p),"%s/%s", g_base, n?n:"");
    FILE* f=fopen(p,"rb");
    if(!f){ if(buf)*buf=0; LOG("  FS_ReadFile '%s' -> not found\n", n?n:"?"); return -1; }
    fseek(f,0,SEEK_END); long sz=ftell(f); fseek(f,0,SEEK_SET);
    char* b=(char*)malloc(sz+1); size_t r=fread(b,1,sz,f); b[r]=0; fclose(f);
    if(buf)*buf=b; LOG("  FS_ReadFile '%s' -> %ld bytes\n", n, (long)r); return (int)r;
}
static void  S_FS_FreeFile(void* p){ free(p); }
static int   S_FS_GetFileList(const char*, const char*, char*, int){ return 0; }
static qboolean S_Append(unsigned long, void*, int){ return qtrue; }
static int   S_ReadSG(unsigned long, void*, int, void**){ return 0; }
static int   S_ReadSGOpt(unsigned long, void*, int, void**){ return 0; }
static void  S_SendConsole(const char*){}
static void  S_DropClient(int, const char*){}
static void  S_SendServer(int, const char* fmt, ...){ (void)fmt; }
static void  S_SetCS(int, const char*){}
static void  S_GetCS(int, char* b, int n){ if(n) b[0]=0; }
static void  S_GetUI(int, char* b, int n){ if(n) b[0]=0; }
static void  S_SetUI(int, const char*){}
static void  S_GetSI(char* b, int n){ if(n) b[0]=0; }
static void  S_SetBrush(gentity_t*, const char*){}
static void  S_trace(trace_t* r, const vec3_t, const vec3_t, const vec3_t, const vec3_t, int, int){ if(r){ memset(r,0,sizeof(*r)); r->fraction=1.0f; } }
static int   S_pointcontents(const vec3_t, int){ return 0; }
static qboolean S_inPVS(const vec3_t, const vec3_t){ return qtrue; }
static qboolean S_inPVS2(const vec3_t, const vec3_t){ return qtrue; }
static void  S_AdjustAreaPortal(gentity_t*, qboolean){}
static qboolean S_AreasConnected(int, int){ return qtrue; }
static void  S_link(gentity_t*){}
static void  S_unlink(gentity_t*){}
static int   S_EntitiesInBox(const vec3_t, const vec3_t, gentity_t**, int){ return 0; }
static qboolean S_EntityContact(const vec3_t, const vec3_t, const gentity_t*){ return qfalse; }
static void* S_Malloc(int n){ return malloc(n); }
static void  S_Free(void* p){ free(p); }

typedef game_export_t* (*GetGameAPI_t)(game_import_t*);
typedef void (*dllEntry_t)(intptr_t (*)(intptr_t,...));
typedef intptr_t (*vmMain_t)(int,...);

int main(int argc, char** argv){
    L = stdout;
    const char* path = (argc>1) ? argv[1] : "./libefgame.so";
    LOG("=== EF-SP M1.0 bridge harness ===\n");
    LOG("dlopen(%s)...\n", path);
    void* h = dlopen(path, RTLD_NOW);
    if(!h){ LOG("FAIL dlopen: %s\n", dlerror()); return 1; }
    LOG("  ok handle=%p\n", h);

    GetGameAPI_t GetGameAPI = (GetGameAPI_t)dlsym(h, "_Z10GetGameAPIP13game_import_t");
    dllEntry_t   dllEntry   = (dllEntry_t)dlsym(h, "_Z8dllEntryPFiizE");
    vmMain_t     vmMain     = (vmMain_t)dlsym(h, "_Z6vmMainiiiiiiiii");
    LOG("dlsym GetGameAPI=%p dllEntry=%p vmMain=%p\n", (void*)GetGameAPI,(void*)dllEntry,(void*)vmMain);
    if(!GetGameAPI){ LOG("FAIL: GetGameAPI not found\n"); return 2; }

    game_import_t gi; memset(&gi,0,sizeof(gi));
    gi.Printf=S_Printf; gi.WriteCam=S_WriteCam; gi.Error=S_Error; gi.Milliseconds=S_Milliseconds;
    gi.cvar=S_cvar; gi.cvar_set=S_cvar_set; gi.Cvar_VariableIntegerValue=S_CvarInt; gi.Cvar_VariableStringBuffer=S_CvarStr;
    gi.argc=S_argc; gi.argv=S_argv;
    gi.FS_FOpenFile=S_FS_FOpenFile; gi.FS_Read=S_FS_Read; gi.FS_Write=S_FS_Write; gi.FS_FCloseFile=S_FS_FCloseFile;
    gi.FS_ReadFile=S_FS_ReadFile; gi.FS_FreeFile=S_FS_FreeFile; gi.FS_GetFileList=S_FS_GetFileList;
    gi.AppendToSaveGame=S_Append; gi.ReadFromSaveGame=S_ReadSG; gi.ReadFromSaveGameOptional=S_ReadSGOpt;
    gi.SendConsoleCommand=S_SendConsole; gi.DropClient=S_DropClient; gi.SendServerCommand=S_SendServer;
    gi.SetConfigstring=S_SetCS; gi.GetConfigstring=S_GetCS; gi.GetUserinfo=S_GetUI; gi.SetUserinfo=S_SetUI; gi.GetServerinfo=S_GetSI;
    gi.SetBrushModel=S_SetBrush; gi.trace=S_trace; gi.pointcontents=S_pointcontents;
    gi.inPVS=S_inPVS; gi.inPVSIgnorePortals=S_inPVS2; gi.AdjustAreaPortalState=S_AdjustAreaPortal; gi.AreasConnected=S_AreasConnected;
    gi.linkentity=S_link; gi.unlinkentity=S_unlink; gi.EntitiesInBox=S_EntitiesInBox; gi.EntityContact=S_EntityContact;
    gi.S_Override=g_sOverride; gi.Malloc=S_Malloc; gi.Free=S_Free;

    LOG("calling GetGameAPI(&gi)...\n");
    game_export_t* ge = GetGameAPI(&gi);
    if(!ge){ LOG("FAIL: GetGameAPI returned NULL\n"); return 3; }
    LOG("  ge=%p apiversion=%d (expected %d) gentitySize=%d num_entities=%d\n",
        (void*)ge, ge->apiversion, GAME_API_VERSION, ge->gentitySize, ge->num_entities);
    if(ge->apiversion != GAME_API_VERSION){ LOG("FAIL: apiversion mismatch\n"); return 4; }
    LOG("PASS: ABI bridge live — GetGameAPI returned a valid game_export_t.\n");

    // Peek: attempt ge->Init with an empty world (stub FS means no real map; we just see how
    // far init runs / where it needs real services). Guarded so the PASS above is already logged.
    if(ge->Init){
        LOG("attempting ge->Init(empty world)...\n");
        ge->Init("_m1test", "", 0, "", 0, 0, 0, (SavedGameJustLoaded_e)0, qfalse);
        LOG("ge->Init returned without crashing.\n");
    }
    if(ge->RunFrame){
        LOG("calling ge->RunFrame() x3...\n");
        for(int i=0;i<3;i++) ge->RunFrame(100*(i+1));
        LOG("ge->RunFrame ticked without crashing.\n");
    }
    LOG("=== harness done ===\n");
    return 0;
}
