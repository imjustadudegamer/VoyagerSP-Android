// sp_ui.h — neutral interface between the engine (cl_*.c, HM headers) and the SP UI
// bridge (sp_ui_bridge.cpp, SP UI headers). EF1's SP UI (libefui.so) is FAKK/Q2 direct-
// struct style: GetUIAPI() returns a uiexport_t; the engine passes a 59-fn uiimport_t.
// This replaces the Holomatch vm/ui.qvm path so the real EF1 main menu is shown.
// Plain int/char types only so both header worlds can include it.
#ifndef SP_UI_H
#define SP_UI_H
#ifdef __cplusplus
extern "C" {
#endif

// ---- SP-side (sp_ui_bridge.cpp): load + drive the SP UI module ----
int  SPUI_Load(void);                 // dlopen libefui.so + GetUIAPI + UI_Init; 1 on success
void SPUI_Shutdown(void);
int  SPUI_IsActive(void);             // 1 once the SP UI module is loaded
void SPUI_Refresh(int realtime);
void SPUI_KeyEvent(int key, int down);// engine passes down-flag; bridge forwards key-DOWN only
void SPUI_MouseEvent(int dx, int dy);
int  SPUI_IsFullscreen(void);         // via UI_GetActiveMenu out-param
void SPUI_SetActiveMenu(const char* menuName);  // "main" / "ingame" / NULL(=off)
int  SPUI_ConsoleCommand(void);
void SPUI_DrawConnect(const char* servername, const char* updateInfoString);

// ---- HM-side accessors (sp_integration.c) the bridge's uiimport adapters call ----
int  SPUI_GetClientState(void);                       // returns clc.state
void SPUI_GetConfigString(int index, char* buf, int size);

// ---- SP map-load screen (sp_integration.c). While a load is in flight the parallel HM
// cgame/ui draw the Holomatch loading splash; the engine suppresses that and shows the
// real per-map levelshot instead. Covers New Game, maptransition, loadtransition, load. ----
void SP_BeginLoad(const char* map);   // mark load start + remember map for the levelshot
void SP_EndLoad(void);                // SP cgame is now active; stop drawing the load screen
int  SP_IsLoading(void);              // 1 while an SP map load is in flight
void SP_DrawLoadingScreen(void);      // draw levelshots/<map> fullscreen (renderer-independent)
// EF1 SP 1:1 loading screen: the cgame drives the animated LCARS load art via cgi_UpdateScreen during CG_INIT.
void SP_PumpLoadScreen(void);         // cgame requested a redraw -> present one cgame-drawn load frame
int  SP_LoadPumping(void);            // 1 while inside SP_PumpLoadScreen (cgame load frame, not static fallback)
void SP_DrawCGameLoad(void);          // re-enter the live cgame -> CG_DrawInformation (bridge, sp_bridge.cpp)
// Save-thumbnail clean capture: hide the menu overlay for the one frame the thumbnail is read back.
void SP_ArmMenuSuppress(void);        // SP_DrawFrame: arm on the capture frame
int  SP_TakeMenuSuppress(void);       // SCR_DrawScreenField: 1 = skip the menu overlay this frame (one-shot)

#ifdef __cplusplus
}
#endif
#endif
