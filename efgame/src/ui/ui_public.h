#ifndef __UI_PUBLIC_H__
#define __UI_PUBLIC_H__


#include "../client/keycodes.h"


#define UI_API_VERSION	2


typedef struct {
	//============== general Quake services ==================

	// print message on the local console
	void		(*Printf)( const char *fmt, ... );

	// abort the game
	void		(*Error)( int level, const char *fmt, ... );

	// console variable interaction
	void		(*Cvar_Set)( const char *name, const char *value );
	float		(*Cvar_VariableValue)( const char *var_name );
	void		(*Cvar_VariableStringBuffer)( const char *var_name, char *buffer, int bufsize );
	void		(*Cvar_SetValue)( const char *var_name, float value );
	void		(*Cvar_Reset)( const char *name );
	void		(*Cvar_Create)( const char *var_name, const char *var_value, int flags );
	void		(*Cvar_InfoStringBuffer)( int bit, char *buffer, int bufsize );

	// console command interaction
	int			(*Argc)( void );
	void		(*Argv)( int n, char *buffer, int bufferLength );
	void		(*Cmd_ExecuteText)( int exec_when, const char *text );
	void		(*Cmd_TokenizeString)( const char *text );

	// filesystem access
	int			(*FS_FOpenFile)( const char *qpath, fileHandle_t *file, fsMode_t mode );
	int 		(*FS_Read)( void *buffer, int len, fileHandle_t f );
	int 		(*FS_Write)( const void *buffer, int len, fileHandle_t f );
	void		(*FS_FCloseFile)( fileHandle_t f );
	int			(*FS_GetFileList)(  const char *path, const char *extension, char *listbuf, int bufsize );
	int			(*FS_ReadFile)( const char *name, void **buf );
	void		(*FS_FreeFile)( void *buf );

	// =========== renderer function calls ================

	qhandle_t	(*R_RegisterModel)( const char *name );			// returns rgb axis if not found
	qhandle_t	(*R_RegisterSkin)( const char *name );			// returns all white if not found
	qhandle_t	(*R_RegisterShader)( const char *name );			// returns white if not found
	qhandle_t	(*R_RegisterShaderNoMip)( const char *name );			// returns white if not found

	// a scene is built up by calls to R_ClearScene and the various R_Add functions.
	// Nothing is drawn until R_RenderScene is called.
	void		(*R_ClearScene)( void );
	void		(*R_AddRefEntityToScene)( const refEntity_t *re );
	void		(*R_AddPolyToScene)( qhandle_t hShader , int numVerts, const polyVert_t *verts );
	void		(*R_AddLightToScene)( const vec3_t org, float intensity, float r, float g, float b );
	void		(*R_RenderScene)( const refdef_t *fd );

	void		(*R_SetColor)( const float *rgba );	// NULL = 1,1,1,1
	void		(*R_DrawStretchPic) ( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );	// 0 = white
	void		(*R_ScissorPic) ( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );	// 0 = white

	// force a screen update, only used during gamestate load
	void		(*UpdateScreen)( void );

	// stuff for savegame screenshots...
	void		(*PrecacheScreenshot)( void );

	//========= model collision ===============

	// R_LerpTag is only valid for md3 models
	void		(*R_LerpTag)( orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, 
						 float frac, const char *tagName );

	// =========== sound function calls ===============

	void		(*S_StartLocalSound)( sfxHandle_t sfxHandle, int channelNum );
	sfxHandle_t	(*S_RegisterSound)( const char* name);
	void		(*S_StartLocalLoopingSound)( sfxHandle_t sfxHandle);


	// =========== getting save game picture ===============
	void	(*DrawStretchRaw) (int x, int y, int w, int h, int cols, int rows, const byte *data, float fLightValue);
	qboolean(*SG_GetSaveImage)( const char *psPathlessBaseName, void *pvAddress );
	void *(*SG_GetSaveGameComment)(const char *psPathlessBaseName);
	qboolean (*SG_ValidateForLoadSaveScreen)(const char *psPathlessBaseName);
	qboolean (*SG_GameAllowedToSaveHere)(qboolean inCamera);
	void (*SG_StoreSaveGameComment)(const char *sComment);
	byte *(*SCR_GetScreenshot)(qboolean *);

	// =========== data shared with the client system =============

	// keyboard and key binding interaction
	void		(*Key_KeynumToStringBuf)( int keynum, char *buf, int buflen );
	void		(*Key_GetBindingBuf)( int keynum, char *buf, int buflen );
	void		(*Key_SetBinding)( int keynum, const char *binding );
	qboolean	(*Key_IsDown)( int keynum );
	qboolean	(*Key_GetOverstrikeMode)( void );
	void		(*Key_SetOverstrikeMode)( qboolean state );
	void		(*Key_ClearStates)( void );
	int			(*Key_GetCatcher)( void );
	void		(*Key_SetCatcher)( int catcher );

	void		(*GetClipboardData)( char *buf, int bufsize );

	void		(*GetGlconfig)( glconfig_t *config );

	connstate_t	(*GetClientState)( void );

	void		(*GetConfigString)( int index, char* buff, int buffsize );

	int			(*Milliseconds)( void );
} uiimport_t;

typedef struct {
	void		(*UI_Init)( int apiVersion, uiimport_t *uiimport );
	void		(*UI_Shutdown)( void );
	void		(*UI_KeyEvent)( int key );
	void		(*UI_MouseEvent)( int dx, int dy );
	void		(*UI_Refresh)( int time );
	void		(*UI_GetActiveMenu)( char **menuname, qboolean *fullscreen );
	void		(*UI_SetActiveMenu)( const char* menuname, const char *menuID);
	qboolean	(*UI_ConsoleCommand)( void );
	void		(*UI_DrawConnect)( const char *servername, const char * updateInfoString );
	void		(*UI_DrawConnectText)( const char *servername, const char * updateInfoString );
	void		(*UI_UpdateConnectionString)( char *string );
	void		(*UI_UpdateConnectionMessageString)( char *string );

} uiexport_t;

uiexport_t *GetUIAPI( void );

#endif
