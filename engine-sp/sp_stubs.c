// sp_stubs.c — headless stubs for engine subsystems common.c references but the
// FS-mount milestone doesn't use yet (client, networking, full server, message boxes).
// The SV_* stubs will be replaced by the real SP server + GetGameAPI bridge next.
#include "q_shared.h"
#include "qcommon.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

// ---- client (dedicated: not used) ----
void CL_CDDialog( void ){}
qboolean CL_CDKeyValidate( const char *key, const char *checksum ){ (void)key;(void)checksum; return qtrue; }
void CL_CharEvent( int key ){ (void)key; }
void CL_Disconnect( qboolean showMainMenu ){ (void)showMainMenu; }
void CL_FlushMemory( void ){}
void CL_ForwardCommandToServer( const char *string ){ (void)string; }
qboolean CL_GameCommand( void ){ return qfalse; }
void CL_Init( void ){}
void CL_InitKeyCommands( void ){}
void CL_JoystickEvent( int axis, int value, int time ){ (void)axis;(void)value;(void)time; }
void CL_KeyEvent( int key, qboolean down, unsigned time ){ (void)key;(void)down;(void)time; }
void CL_MouseEvent( int dx, int dy, int time ){ (void)dx;(void)dy;(void)time; }
void CL_PacketEvent( netadr_t from, msg_t *msg ){ (void)from;(void)msg; }
void CL_Shutdown( char *finalmsg, qboolean disconnect, qboolean quit ){ (void)finalmsg;(void)disconnect;(void)quit; }
void CL_StartHunkUsers( qboolean rendererOnly ){ (void)rendererOnly; }
void IN_Frame( void ){}
void Key_WriteBindings( fileHandle_t f ){ (void)f; }
void S_ClearSoundBuffer( void ){}
qboolean UI_GameCommand( void ){ return qfalse; }

// ---- networking (headless: no net) ----
void Netchan_Init( int qport ){ (void)qport; }
void NET_FlushPacketQueue( void ){}
qboolean NET_GetLoopPacket( netsrc_t sock, netadr_t *net_from, msg_t *net_message ){ (void)sock;(void)net_from;(void)net_message; return qfalse; }
void NET_Restart_f( void ){}
void NET_Sleep( int msec ){ (void)msec; }

// ---- server (placeholder — real SP server + GetGameAPI bridge replaces these) ----
void SV_Frame( int msec ){ (void)msec; }
int  SV_FrameMsec( void ){ return 1000; }
qboolean SV_GameCommand( void ){ return qfalse; }
void SV_Init( void ){}
void SV_PacketEvent( netadr_t from, msg_t *msg ){ (void)from;(void)msg; }
int  SV_SendQueuedPackets( void ){ return -1; }
void SV_Shutdown( char *finalmsg ){ (void)finalmsg; }
void SV_ShutdownGameProgs( void ){}

// ---- SDL message boxes (Sys_Dialog; irrelevant headless) ----
const char *SDL_GetError( void ){ return ""; }
int SDL_ShowMessageBox( const void *data, int *buttonid ){ (void)data; if(buttonid)*buttonid=0; return -1; }
int SDL_ShowSimpleMessageBox( unsigned int flags, const char *title, const char *message, void *window ){ (void)flags;(void)title;(void)message;(void)window; return -1; }

// ---- system layer (minimal real impls) ----
void Sys_Print( const char *msg ){ fputs( msg, stdout ); fflush(stdout); }
void QDECL Sys_Error( const char *error, ... ){ va_list a; va_start(a,error); fprintf(stderr,"\nSys_Error: "); vfprintf(stderr,error,a); va_end(a); fprintf(stderr,"\n"); exit(1); }
void Sys_Quit( void ){ exit(0); }
void Sys_Init( void ){}
char *Sys_DefaultInstallPath( void ){ static char p[]="/data/local/tmp/efsp"; return p; }
char *Sys_ConsoleInput( void ){ return NULL; }
cpuFeatures_t Sys_GetProcessorFeatures( void ){ return (cpuFeatures_t)0; }
void Sys_InitPIDFile( const char *gamedir ){ (void)gamedir; }
void Sys_RemovePIDFile( const char *gamedir ){ (void)gamedir; }
void Sys_UnloadDll( void *dllHandle ){ if(dllHandle) dlclose(dllHandle); }
