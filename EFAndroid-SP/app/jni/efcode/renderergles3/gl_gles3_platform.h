/*
===========================================================================

GLES 3.2 platform layer for the renderer_gles3 module.

See gl_gles3_platform.c for what this owns and why it does not use the
client's Vulkan window hooks (ri.VKimp_*).

===========================================================================
*/
#ifndef GL_GLES3_PLATFORM_H
#define GL_GLES3_PLATFORM_H

#include "../qcommon/q_shared.h"
#include "../renderercommon/tr_public.h"

// Create the window and an ES 3.2 context, resolve the GL entry points and
// fill *config with this engine's glconfig_t. qfalse means no usable context
// (the module must then not pretend to render).
qboolean GL3_PlatformInit( glconfig_t *config );

// Present the default framebuffer.
void GL3_PlatformSwapBuffers( void );

// Tear the context/window down; unloadLibrary is the renderer's
// REF_UNLOAD_DLL flag (SDL owns the GL libraries either way).
void GL3_PlatformShutdown( qboolean unloadLibrary );

#endif // GL_GLES3_PLATFORM_H
