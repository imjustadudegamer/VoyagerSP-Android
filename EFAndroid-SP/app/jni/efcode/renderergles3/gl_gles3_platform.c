/*
===========================================================================

GLES 3.2 platform layer for the renderer_gles3 module.

Ported from the author's EF2 port (renderergles3/gl_gles3_platform.c), reduced
to what this engine needs and rewired to this engine's data:

  * window + GLES 3.2 context are created here with SDL (the Vulkan renderer's
    window comes from the client through ri.VKimp_Init, which is Vulkan-only
    and therefore must NOT be used by this module);
  * the GL entry points are resolved through the ported qgl_gles3 loader
    (`qgl_gles3.c`), not through link-time GL symbols -- so the module builds
    and loads on a machine with no GLES headers/libraries at link time;
  * the result is written into this engine's `glconfig_t`
    (renderercommon/tr_types.h), the same struct the Vulkan renderer fills.

EF2-specific pieces that are deliberately not here: hwtex/ASTC texture
transcode, the "first light" probe, the EGL-over-ANativeWindow path (SDL_GL
does that internally on Android) and the render thread.

NOT yet called by anything: the drawing backend that would use this context is
the remaining work (see AUDIT.md). GL3_PlatformInit is exercised the moment that
backend lands, and it is compiled and linked on both targets today.

===========================================================================
*/
#include "gl_gles3_platform.h"
#include "qgl_gles3.h"

#include <SDL.h>
#include <string.h>

// The module's private copy of the engine import table (gl3_renderer.c).
extern refimport_t ri;

static SDL_Window    *gl3_window = NULL;
static SDL_GLContext  gl3_context = NULL;
static qboolean       gl3_subsystemInited = qfalse;
static int            gl3_drawableWidth = 0;	// last measured drawable, for GL3_PlatformDrawableSize
static int            gl3_drawableHeight = 0;

static void GL3_LogToEngine( const char *line )
{
	ri.Printf( PRINT_ALL, "%s", line );
}

static void *GL3_ResolveProc( const char *name )
{
	// SDL_GL_GetProcAddress covers core + extension entry points on both
	// platforms (wgl/glX/EGL under the hood); eglGetProcAddress is only needed
	// for the handful of EGL-only names, which the loader treats as optional.
	return (void *)SDL_GL_GetProcAddress( name );
}

static void GL3_ModeFromCvars( int *width, int *height, qboolean *fullscreen )
{
	int mode;
	int w, h;

	mode = ri.Cvar_VariableIntegerValue( "r_mode" );
	w = ri.Cvar_VariableIntegerValue( "r_customwidth" );
	h = ri.Cvar_VariableIntegerValue( "r_customheight" );
	*fullscreen = ri.Cvar_VariableIntegerValue( "r_fullscreen" ) ? qtrue : qfalse;

	if ( mode < -1 ) {
		// -2 = native desktop size
		SDL_DisplayMode dm;
		if ( SDL_GetDesktopDisplayMode( 0, &dm ) == 0 ) {
			w = dm.w;
			h = dm.h;
		}
	} else if ( mode == -1 || w < 320 || h < 240 ) {
		// The Vulkan path keeps windowed at 1280x720 unless told otherwise; use
		// the same default so both renderers produce the same window on the
		// same config.
		w = 1280;
		h = 720;
	}
	*width = w;
	*height = h;
}

qboolean GL3_PlatformInit( glconfig_t *config )
{
	SDL_DisplayMode dm;
	const char *str;
	int width = 0, height = 0, dw = 0, dh = 0;
	qboolean fullscreen = qfalse;
	Uint32 flags;

	if ( !SDL_WasInit( SDL_INIT_VIDEO ) ) {
		if ( SDL_Init( SDL_INIT_VIDEO ) != 0 ) {
			ri.Printf( PRINT_WARNING, "GLES3: SDL_Init(VIDEO) failed: %s\n", SDL_GetError() );
			return qfalse;
		}
	}
	gl3_subsystemInited = qtrue;

	SDL_SetHint( SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight" );

	if ( SDL_GetDesktopDisplayMode( 0, &dm ) != 0 ) {
		dm.w = 1280;
		dm.h = 720;
		dm.refresh_rate = 60;
	}

#ifdef __ANDROID__
	// One window, always fullscreen at the device resolution (the Vulkan path
	// does the same; there is nothing to resize on a phone).
	width = dm.w;
	height = dm.h;
	fullscreen = qtrue;
#else
	GL3_ModeFromCvars( &width, &height, &fullscreen );
#endif

	SDL_GL_ResetAttributes();
	if ( SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES ) != 0 ||
	     SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 ) != 0 ||
	     SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 ) != 0 ) {
		ri.Printf( PRINT_WARNING, "GLES3: SDL would not accept an ES 3.2 context request: %s\n", SDL_GetError() );
		return qfalse;
	}
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_STEREO, 0 );

	flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
	if ( fullscreen )
		flags |= SDL_WINDOW_FULLSCREEN;
	else if ( ri.Cvar_VariableIntegerValue( "r_allowResize" ) )
		flags |= SDL_WINDOW_RESIZABLE;

	gl3_window = SDL_CreateWindow( CLIENT_WINDOW_TITLE,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height, flags );
	if ( !gl3_window ) {
		ri.Printf( PRINT_WARNING, "GLES3: SDL_CreateWindow(OPENGL) failed: %s\n", SDL_GetError() );
		return qfalse;
	}

	gl3_context = SDL_GL_CreateContext( gl3_window );
	if ( !gl3_context ) {
		ri.Printf( PRINT_WARNING, "GLES3: SDL_GL_CreateContext(ES 3.2) failed: %s\n", SDL_GetError() );
		SDL_DestroyWindow( gl3_window );
		gl3_window = NULL;
		return qfalse;
	}

	if ( SDL_GL_MakeCurrent( gl3_window, gl3_context ) != 0 ) {
		ri.Printf( PRINT_WARNING, "GLES3: SDL_GL_MakeCurrent failed: %s\n", SDL_GetError() );
		SDL_GL_DeleteContext( gl3_context );
		gl3_context = NULL;
		SDL_DestroyWindow( gl3_window );
		gl3_window = NULL;
		return qfalse;
	}

	SDL_GL_SetSwapInterval( ri.Cvar_VariableIntegerValue( "r_swapInterval" ) ? 1 : 0 );

	// Resolve the whole GLES 3.2 entry-point set before anyone draws. The
	// loader reports core entry points that the driver did not expose; that is
	// a hard failure for a renderer that needs the core set.
	QGL_GLES3_SetGetProcAddress( GL3_ResolveProc );
	QGL_GLES3_SetLogger( GL3_LogToEngine );
	{
		int staged = 0;
		int missingCore = QGL_GLES3_LoadProcs( &staged );

		QGL_GLES3_QueryVersion();
		QGL_GLES3_LogDeviceInfo();

		if ( missingCore > 0 ) {
			ri.Printf( PRINT_WARNING, "GLES3: %d required core entry point(s) missing: %s\n",
				missingCore, QGL_GLES3_CoreMissingList() );
			GL3_PlatformShutdown( qfalse );
			return qfalse;
		}
		(void)staged;
	}

	if ( !qglGetString || !qglGetIntegerv ) {
		ri.Printf( PRINT_WARNING, "GLES3: the context exposes no GL core queries\n" );
		return qfalse;
	}

	SDL_GL_GetDrawableSize( gl3_window, &dw, &dh );
	if ( dw <= 0 || dh <= 0 )
		SDL_GetWindowSize( gl3_window, &dw, &dh );
	if ( dw > 0 && dh > 0 ) {
		gl3_drawableWidth = dw;
		gl3_drawableHeight = dh;
	}

	Com_Memset( config, 0, sizeof( *config ) );
	config->vidWidth = dw;
	config->vidHeight = dh;
	config->windowAspect = dh ? (float)dw / (float)dh : 4.0f / 3.0f;
	config->displayFrequency = dm.refresh_rate;
	config->isFullscreen = fullscreen;
	config->stereoEnabled = qfalse;
	// GLES has no hardware gamma ramp; the renderer's gamma post-pass covers it
	// (same contract as the Vulkan backend, which reports qfalse here too).
	config->deviceSupportsGamma = qfalse;
	config->colorBits = 32;
	config->depthBits = 24;
	config->stencilBits = 8;
	config->driverType = GLDRV_ICD;
	config->hardwareType = GLHW_GENERIC;
	config->textureCompression = TC_NONE;
	config->textureEnvAddAvailable = qfalse;
#ifdef ELITEFORCE
	config->textureFilterAnisotropicAvailable = qfalse;
#endif

	qglGetIntegerv( GL_MAX_TEXTURE_SIZE, &config->maxTextureSize );
	qglGetIntegerv( GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &config->numTextureUnits );

	str = (const char *)qglGetString( GL_VENDOR );
	Q_strncpyz( config->vendor_string, str ? str : "GLES", sizeof( config->vendor_string ) );
	str = (const char *)qglGetString( GL_RENDERER );
	Q_strncpyz( config->renderer_string, str ? str : "GLES", sizeof( config->renderer_string ) );
	str = (const char *)qglGetString( GL_VERSION );
	Q_strncpyz( config->version_string, str ? str : "GLES", sizeof( config->version_string ) );
	str = (const char *)qglGetString( GL_EXTENSIONS );
	Q_strncpyz( config->extensions_string, str ? str : "", sizeof( config->extensions_string ) );

	ri.Printf( PRINT_ALL, "GLES3: %dx%d %s, renderer \"%s\", GL %s\n",
		dw, dh, fullscreen ? "fullscreen" : "windowed",
		config->renderer_string, config->version_string );

	// Input is attached to the same window the renderer draws into.
	ri.IN_Init( gl3_window );

	return qtrue;
}

// Recompute the desired window size from the same cvars the first init used and
// apply it to the live window. This is the r_mode / r_fullscreen / vid_restart
// path: the engine keeps the window (REF_KEEP_WINDOW) and expects the renderer
// to resize.
void GL3_PlatformResync( glconfig_t *config )
{
	int width = 0, height = 0;
	qboolean fullscreen = qfalse;
	int dw = 0, dh = 0;

	if ( !gl3_window )
		return;

#ifdef __ANDROID__
	{
		SDL_DisplayMode dm;
		if ( SDL_GetDesktopDisplayMode( 0, &dm ) == 0 ) {
			width = dm.w;
			height = dm.h;
		}
		fullscreen = qtrue;
	}
#else
	GL3_ModeFromCvars( &width, &height, &fullscreen );
#endif

	if ( width > 0 && height > 0 ) {
		SDL_SetWindowFullscreen( gl3_window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0 );
		SDL_SetWindowSize( gl3_window, width, height );
	}

	SDL_GL_GetDrawableSize( gl3_window, &dw, &dh );
	if ( dw <= 0 || dh <= 0 )
		SDL_GetWindowSize( gl3_window, &dw, &dh );
	if ( dw > 0 && dh > 0 ) {
		gl3_drawableWidth = dw;
		gl3_drawableHeight = dh;
	}

	if ( dw > 0 && dh > 0 ) {
		config->vidWidth = dw;
		config->vidHeight = dh;
		config->windowAspect = (float)dw / (float)dh;
		config->isFullscreen = fullscreen;
	}
}

void GL3_PlatformDrawableSize( int *width, int *height )
{
	int dw = 0, dh = 0;

	if ( gl3_window )
		SDL_GL_GetDrawableSize( gl3_window, &dw, &dh );
	if ( dw <= 0 || dh <= 0 ) {
		dw = gl3_drawableWidth;
		dh = gl3_drawableHeight;
	}
	if ( width ) *width = dw;
	if ( height ) *height = dh;
}

void GL3_PlatformSwapBuffers( void )
{
	if ( gl3_window && gl3_context )
		SDL_GL_SwapWindow( gl3_window );
}

void GL3_PlatformShutdown( qboolean unloadLibrary )
{
	if ( gl3_window || gl3_context ) {
		ri.IN_Shutdown();
	}
	if ( gl3_context ) {
		SDL_GL_DeleteContext( gl3_context );
		gl3_context = NULL;
	}
	if ( gl3_window ) {
		SDL_DestroyWindow( gl3_window );
		gl3_window = NULL;
	}
	if ( gl3_subsystemInited ) {
		SDL_QuitSubSystem( SDL_INIT_VIDEO );
		gl3_subsystemInited = qfalse;
	}
	// SDL itself owns the EGL/GLES libraries it opened; there is nothing extra
	// to unload here. The parameter exists so the call site matches the
	// renderer's REF_UNLOAD_DLL convention.
	(void)unloadLibrary;
}
