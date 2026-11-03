/*
===========================================================================

renderer_gles3 -- the second renderer module.

THE CONTRACT (same as the Vulkan module, see AUDIT.md §6):
  * the client dlopen()s this module once, at CL_InitRef, before any UI exists,
    and calls GetRefAPI(REF_API_VERSION, &ri);
  * everything this module needs from the engine comes through that import
    table (`ri`); it links no engine symbol and `dlopen` stays RTLD_LOCAL, so
    the private Com_Printf, Cvar_ and Q_ helper copies compiled into every
    module cannot
    interpose on one another;
  * this module is never the default. Even when it is complete the client will
    load Vulkan unless the user picked gles3 (Renderer row / setrenderer), and
    a NULL return from GetRefAPI makes the client unload it, reset
    `cl_renderer` to the compiled default and retry -- i.e. an incomplete
    backend degrades to Vulkan instead of crashing.

STATE: the GL layer (qgl_gles3 loader, SDL/GLES 3.2 context layer) and the GLSL
ES 3.20 shader set are ported and compile; the drawing backend is not written
yet (see AUDIT.md §5). GetRefAPI therefore returns NULL with an explicit
message, and renderer_backend_ready() returns 0, which keeps the module out of
cl_rendererModules and therefore out of the video menu.

===========================================================================
*/
#include "../qcommon/q_shared.h"
#include <stdarg.h>
#include "../renderercommon/tr_public.h"
#include "gl_gles3_platform.h"
#include "gl3_shaders.h"

// This module's private copy of the engine import table. Filled by GetRefAPI;
// every other file in the module (gl_gles3_platform.c) uses it.
refimport_t ri;

/*
====================
renderer_backend_ready

Exported for the client's module probe (CL_PublishRendererModules): a backend
is only offered in the UI once this returns qtrue. The Vulkan module returns
qtrue from its own copy of this function.
====================
*/
Q_EXPORT int renderer_backend_ready( void )
{
	return 0;
}

// Standalone-module shims. q_shared.c calls Com_Error/Com_Printf (its string
// helpers report impossible formatting), and a module cannot link the engine's
// qcommon -- exactly the situation renderervk/tr_init.c handles for the Vulkan
// module. Re-expressed over this module's import table.
void QDECL Com_Printf( const char *fmt, ... )
{
	char	buf[1024];	// qcommon's MAXPRINTMSG; kept local so the module has no engine header dep
	va_list	argptr;

	va_start( argptr, fmt );
	Q_vsnprintf( buf, sizeof( buf ), fmt, argptr );
	va_end( argptr );

	ri.Printf( PRINT_ALL, "%s", buf );
}

void QDECL Com_Error( int code, const char *fmt, ... )
{
	char	buf[1024];	// qcommon's MAXPRINTMSG; kept local so the module has no engine header dep
	va_list	argptr;

	va_start( argptr, fmt );
	Q_vsnprintf( buf, sizeof( buf ), fmt, argptr );
	va_end( argptr );

	ri.Error( code, "%s", buf );
}

/*
====================
GL3_ShaderSources / GL3_ShaderCount

The embedded GLSL ES 3.20 set, for the backend that will compile it with
glCompileShader(). Kept non-static and referenced here so the shader strings
are always part of the module image.
====================
*/
const gl3_shader_source_t *GL3_ShaderSources( int *count )
{
	if ( count )
		*count = GL3_SHADER_COUNT;
	return gl3_shader_sources;
}

/*
====================
GetRefAPI

apiVersion / rimp exactly as the client passes them (EF1's refexport_t /
refimport_t, REF_API_VERSION in renderercommon/tr_public.h). Returns NULL until
the backend exists; the client treats that as a failed load.
====================
*/
Q_EXPORT refexport_t *QDECL GetRefAPI( int apiVersion, refimport_t *rimp )
{
	static qboolean warned = qfalse;

	ri = *rimp;

	if ( apiVersion != REF_API_VERSION ) {
		ri.Printf( PRINT_ALL, "Mismatched REF_API_VERSION: expected %i, got %i\n",
			REF_API_VERSION, apiVersion );
		return NULL;
	}

	if ( !warned ) {
		warned = qtrue;
		ri.Printf( PRINT_WARNING,
			"renderer_gles3: the GLES 3.2 GL layer and shaders are present, but the drawing\n"
			"backend is not implemented yet (see renderergles3/AUDIT.md). Refusing to start so\n"
			"the client falls back to the Vulkan renderer.\n" );
	}

	return NULL;
}
