// sp_headless_main.c — first engine-fork milestone: bring up the reused HM qcommon
// headless (dedicated-style) far enough to mount the real SP paks via FS, then (next)
// CM_LoadMap a real map and hand its entity string to the SP game module's ge->Init.
//
// This reuses the heavily-tweaked HM engine's FS/CM/cvar/cmd/common as-is.

#include "q_shared.h"
#include "qcommon.h"

int main(int argc, char** argv){
    // Point the filesystem at the on-device SP data; run dedicated (no client/renderer).
    static char cmdline[] =
        "+set fs_basepath /data/local/tmp/efsp "
        "+set fs_homepath /data/local/tmp/efsp "
        "+set fs_game baseEF "
        "+set dedicated 1 "
        "+set sv_pure 0";

    Com_Printf("=== EF-SP headless engine bring-up ===\n");
    Com_Init(cmdline);

    Com_Printf("FS up. Loaded pak names: %s\n", FS_LoadedPakNames());

    // Load the SP game module and spawn a real campaign map (entities + ICARUS).
    extern qboolean SP_LoadGame( const char *solib );
    extern qboolean SP_SpawnServer( const char *mapname );
    extern void     SP_RunFrames( int n );

    extern qboolean SP_StartClient( void );
    extern void     SP_ClientFrames( int n );

    const char *map = (argc>1) ? argv[1] : "borg1";
    if( SP_LoadGame("/data/local/tmp/efsp/libefgame.so") ){
        SP_SpawnServer( map );
        SP_RunFrames( 3 );          // let the world settle
        if( SP_StartClient() ){     // connect player + CG_INIT
            SP_ClientFrames( 5 );   // drive cgame draw frames (stub renderer)
        }
    }
    Com_Printf("=== SP headless run complete ===\n");
    return 0;
}
