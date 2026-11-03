/*
===========================================================================

renderer_gles3 -- GLES 3.2 drawing backend.

This file implements the same backend interface the Vulkan renderer exposes to
the renderer front end (`vk_*` entry points, `Vk_Pipeline_Def`, the `vk`
state struct), in OpenGL ES 3.2 terms. The EF1 front end (`renderervk/tr_*.c`,
already correct for this engine's BSP/MD3/MDR/IQM/shader data) is compiled into
this module unchanged, so both renderers drive identical front-end logic; only
the drawing layer differs. Nothing here is used by the Vulkan module or by the
client.

Design notes (why the code looks the way it does):

  * Pipelines. The front end builds a `Vk_Pipeline_Def` per shader stage and
    expects a stable integer index for it. This backend keeps the same registry
    (`vk.pipelines[]`), only the "compile" step is a GL program, created lazily
    on first bind and cached by (shader_type, fog_stage).
  * Uniforms. The front end packs a `vkUniform_t` per draw into the mapped
    staging buffer (`VK_PushUniform`) and records the offset. This backend
    uploads that block to one std140 uniform buffer at binding 0; the GLSL was
    converted from the Vulkan sources so its block layout is identical
    (all-vec4 members). The MVP stays a separate `mat4` uniform, exactly like
    the Vulkan push constant.
  * Depth. The renderer front end is compiled with USE_REVERSED_DEPTH (it is
    defined in vk.h): the projection maps the near plane to NDC z=1 and the far
    plane to z=0. GL is fed the same matrices (no Vulkan Y flip), depth is
    cleared to 0 and the comparison is GL_GEQUAL. The 2D transform flips Y for
    GL's bottom-left NDC (the Vulkan version does not).
  * Textures. `image_t::handle` carries the GL texture name and
    `image_t::descriptor` an opaque token (the same name) so the front end's
    `vk_update_descriptor(index, image->descriptor)` calls can be decoded back
    to a texture unit at draw time without touching renderercommon/tr_*.h.
  * Render targets. One offscreen colour texture + depth/stencil renderbuffer
    is the main target; a gamma composite pass draws it to the default
    framebuffer at the end of the frame (parity with the Vulkan gamma pass).
    A second target of the same shape backs the screenMap/mirror pass.
  * Not implemented yet, and stated as such in AUDIT.md: bloom blur chain,
    MSAA resolve, r_smp render thread, flares (needs a GLES depth-visibility
    test), and the dynamic device/context-loss recovery the Vulkan module has.

===========================================================================
*/
#include "../renderervk/tr_local.h"	// pulls in the GLES headers for this module
#include "gl_gles3_platform.h"
#include "qgl_gles3.h"
#include "gl3_shaders.h"

// ---------------------------------------------------------------------------
// State
// ---------------------------------------------------------------------------

typedef struct {
	int			shader_type;
	int			fog_stage;
	GLuint		program;

	int			loc_mvp;
	int			loc_texture0;
	int			loc_texture1;
	int			loc_texture2;
	int			loc_fog_texture;

	int			loc_alpha_test_func;
	int			loc_alpha_test_value;
	int			loc_depth_fragment;
	int			loc_alpha_to_coverage;
	int			loc_color_mode;
	int			loc_abs_light;
	int			loc_tex_mode;
	int			loc_discard_mode;
	int			loc_identity_color;
	int			loc_identity_alpha;
	int			loc_acff;
} gl3_program_t;

#define GL3_MAX_PROGRAMS	64

static gl3_program_t	gl3_programs[GL3_MAX_PROGRAMS];
static int				gl3_program_count;

static vk_tess_t		gl3_cmd;			// the one command buffer state
static byte				*gl3_uniform_stage;	// mapped uniform staging (VK_PushUniform)
#define GL3_UNIFORM_STAGE_SIZE	(4 * 1024 * 1024)

static float			gl3_mvp[16];
static float			gl3_mvp_default[16];

static GLuint			gl3_vao;
static GLuint			gl3_vbo;
static GLuint			gl3_ibo;
static GLuint			gl3_ubo;

static GLuint			gl3_main_fbo;
static GLuint			gl3_main_color;
static GLuint			gl3_main_depth;
static GLuint			gl3_screenmap_fbo;
static GLuint			gl3_screenmap_color;
static GLuint			gl3_screenmap_depth;

static GLuint			gl3_gamma_program;
static GLuint			gl3_gamma_vao;
static int				gl3_gamma_loc[8];	// texture0, gamma, obScale, greyscale, dither, depth_r/g/b

static int				gl3_pipeline;			// def index currently bound
static uint32_t			gl3_geometry_flags;
static qboolean			gl3_initialized;
static qboolean			gl3_frame_active;
static qboolean			gl3_screenmap_pending;
static vec4_t			gl3_clear_color;
static qboolean			gl3_bloom_warned;
static qboolean			gl3_smp_warned;
static int				gl3_width, gl3_height;
static qboolean			gl3_backend_ok;

// First-run diagnostics (see LAUNCH-TEST.md).
static int				gl3_gl_errors;			// errors reported so far (capped)
static int				gl3_first_frame_checked;
static qboolean			gl3_context_lost;
static int				gl3_texture_count;
static qboolean			gl3_selftest_registered;

// The three device-loss entry points are the module surface the client resolves
// with dlsym. For GLES the "device lost" condition is a lost GL context, which
// the error sweep below recognises (GL_CONTEXT_LOST). When it happens the flag
// is raised; the client's CL_RecoverDeviceLost path then calls re.Shutdown ->
// RE_Shutdown(REF_UNLOAD_DLL) -> vk_shutdown -> GL3_PlatformShutdown (which
// destroys the SDL window and its EGL context) and CL_InitRenderer recreates
// everything through InitOpenGL. That is the EGL context recreation story: SDL
// owns the EGL context, so the recovery is "tear the SDL context down and let
// InitOpenGL build a new one", driven by the existing client loop.
qboolean vk_device_lost( void )
{
	return gl3_context_lost;
}

void vk_clear_device_lost( void )
{
	gl3_context_lost = qfalse;
}

int vk_device_lost_report( char *out, int outSize )
{
	int n;

	if ( !out || outSize <= 0 )
		return 0;

	n = Com_sprintf( out, outSize,
		"renderer       : GLES 3.2 (renderer_gles3)\n"
		"GL vendor      : %s\n"
		"GL renderer    : %s\n"
		"GL version     : %s\n"
		"context lost   : %s\n"
		"frame size     : %dx%d\n"
		"programs       : %d\n"
		"textures       : %d\n",
		qglGetString ? (const char *)qglGetString( GL_VENDOR ) : "(null)",
		qglGetString ? (const char *)qglGetString( GL_RENDERER ) : "(null)",
		qglGetString ? (const char *)qglGetString( GL_VERSION ) : "(null)",
		gl3_context_lost ? "yes" : "no",
		gl3_width, gl3_height, gl3_program_count, gl3_texture_count );
	return n;
}

// ---------------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Diagnostics
//
// The user is the first person to run this renderer, so every failure must
// name a source site and the GL error must be visible in the log. GL3_CHECK
// reports the first errors with file:line and then throttles itself: drivers
// can repeat the same error every frame and the log is more useful bounded.
// ---------------------------------------------------------------------------

#define GL3_MAX_GL_ERRORS 16

static const char *GL3_ErrorName( GLenum err )
{
	switch ( err ) {
	case GL_NO_ERROR: return "GL_NO_ERROR";
	case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
	case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
	case 0x0507: return "GL_CONTEXT_LOST";	// GL_KHR_robustness / ES 3.2
	default: return "GL_UNKNOWN_ERROR";
	}
}

static void GL3_CheckErrors_( const char *where, const char *file, int line )
{
	GLenum err;

	if ( gl3_gl_errors >= GL3_MAX_GL_ERRORS )
		return;

	while ( ( err = qglGetError() ) != GL_NO_ERROR ) {
		if ( err == 0x0507 ) {	// GL_CONTEXT_LOST: let the client recover
			gl3_context_lost = qtrue;
		}
		if ( gl3_gl_errors < GL3_MAX_GL_ERRORS ) {
			ri.Printf( PRINT_WARNING, "GLES3: %s at %s:%i\n", GL3_ErrorName( err ), file, line );
			gl3_gl_errors++;
			if ( gl3_gl_errors == GL3_MAX_GL_ERRORS )
				ri.Printf( PRINT_WARNING, "GLES3: further GL errors suppressed\n" );
		}
		(void)where;
	}
}

#define GL3_CHECK( where ) GL3_CheckErrors_( ( where ), __FILE__, __LINE__ )

static void GL3_LogGLInfo( void )
{
	const char *vendor = qglGetString ? (const char *)qglGetString( GL_VENDOR ) : NULL;
	const char *renderer = qglGetString ? (const char *)qglGetString( GL_RENDERER ) : NULL;
	const char *version = qglGetString ? (const char *)qglGetString( GL_VERSION ) : NULL;
	const char *ext = qglGetString ? (const char *)qglGetString( GL_EXTENSIONS ) : NULL;

	ri.Printf( PRINT_ALL, "GLES3: vendor   : %s\n", vendor ? vendor : "(null)" );
	ri.Printf( PRINT_ALL, "GLES3: renderer : %s\n", renderer ? renderer : "(null)" );
	ri.Printf( PRINT_ALL, "GLES3: version  : %s (ES %d.%d)\n",
		version ? version : "(null)", qglesMajorVersion, qglesMinorVersion );
	ri.Printf( PRINT_ALL, "GLES3: config   : %dx%d depth=%d stencil=%d multisample=%d (samples=%d) fbo=%d\n",
		gl3_width, gl3_height, glConfig.depthBits, glConfig.stencilBits,
		r_ext_multisample ? r_ext_multisample->integer : 0, 1, r_fbo ? r_fbo->integer : 1 );
	if ( ext ) {
		char buf[512];
		Q_strncpyz( buf, ext, sizeof( buf ) );
		ri.Printf( PRINT_ALL, "GLES3: extensions: %s%s\n", buf,
			strlen( ext ) >= sizeof( buf ) - 1 ? " ..." : "" );
	}
}

/*
====================
GL3_SelfTest_f

`gles3_selftest` -- everything this backend can verify without rendering a
frame: the driver-reported uniform block layout against the C struct, the
attribute locations the programs were compiled with, the sampler units, the
framebuffer completeness and the object counts.
====================
*/
static void GL3_SelfTest_f( void )
{
	int i;

	ri.Printf( PRINT_ALL, "GLES3 selftest: ---------- begins ----------\n" );
	GL3_LogGLInfo();

	ri.Printf( PRINT_ALL, "GLES3 selftest: objects: programs=%d textures=%d fbo=%u screenmap_fbo=%u vbo=%u ibo=%u ubo=%u vao=%u\n",
		gl3_program_count, gl3_texture_count,
		gl3_main_fbo, gl3_screenmap_fbo, gl3_vbo, gl3_ibo, gl3_ubo, gl3_vao );

	for ( i = 0; i < gl3_program_count; i++ ) {
		gl3_program_t *p = &gl3_programs[i];
		ri.Printf( PRINT_ALL, "GLES3 selftest: program %d: type=%d fog=%d id=%u mvp=%d tex=%d/%d/%d fogtex=%d\n",
			i, p->shader_type, p->fog_stage, p->program, p->loc_mvp,
			p->loc_texture0, p->loc_texture1, p->loc_texture2, p->loc_fog_texture );
	}

	// Driver-reported UBO layout vs the C struct the front end fills.
	if ( gl3_program_count > 0 ) {
		GLuint prog = gl3_programs[0].program;
		GLint block_index = -1, data_size = 0, uniform_count = 0;
		GLint loc;

		GLuint block = qglGetUniformBlockIndex( prog, "UBO" );
		if ( block != GL_INVALID_INDEX ) {
			qglGetActiveUniformBlockiv( prog, block, GL_UNIFORM_BLOCK_DATA_SIZE, &data_size );
			qglGetActiveUniformBlockiv( prog, block, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &uniform_count );
			block_index = (GLint)block;
		}
		ri.Printf( PRINT_ALL, "GLES3 selftest: UBO block=%d data_size=%d expected=%d (delta=%d) uniforms=%d\n",
			block_index, data_size, (int)sizeof( vkUniform_t ),
			data_size - (int)sizeof( vkUniform_t ), uniform_count );

		loc = qglGetAttribLocation( prog, "in_position" );
		ri.Printf( PRINT_ALL, "GLES3 selftest: attrib in_position=%d (expect 0)\n", loc );
		loc = qglGetAttribLocation( prog, "in_color0" );
		ri.Printf( PRINT_ALL, "GLES3 selftest: attrib in_color0=%d (expect 1)\n", loc );
		loc = qglGetAttribLocation( prog, "in_tex_coord0" );
		ri.Printf( PRINT_ALL, "GLES3 selftest: attrib in_tex_coord0=%d (expect 2)\n", loc );
		loc = qglGetAttribLocation( prog, "in_normal" );
		ri.Printf( PRINT_ALL, "GLES3 selftest: attrib in_normal=%d (expect 5 or -1)\n", loc );
	}

	if ( gl3_main_fbo ) {
		qglBindFramebuffer( GL_FRAMEBUFFER, gl3_main_fbo );
		ri.Printf( PRINT_ALL, "GLES3 selftest: main FBO status=%s\n",
			qglCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE ? "complete" : "INCOMPLETE" );
	}
	if ( gl3_screenmap_fbo ) {
		qglBindFramebuffer( GL_FRAMEBUFFER, gl3_screenmap_fbo );
		ri.Printf( PRINT_ALL, "GLES3 selftest: screenMap FBO status=%s\n",
			qglCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE ? "complete" : "INCOMPLETE" );
	}
	qglBindFramebuffer( GL_FRAMEBUFFER, 0 );

	GL3_CHECK( "selftest" );
	ri.Printf( PRINT_ALL, "GLES3 selftest: ---------- ends ----------\n" );
}

static GLuint GL3_TextureFromDescriptor( VkDescriptorSet descriptor )
{
	return (GLuint)(uintptr_t)descriptor;
}

static VkDescriptorSet GL3_DescriptorFromTexture( GLuint texture )
{
	return (VkDescriptorSet)(uintptr_t)texture;
}

static const char *GL3_ShaderSource( const char *name )
{
	int i;

	for ( i = 0; i < GL3_SHADER_COUNT; i++ ) {
		if ( !Q_stricmp( gl3_shader_sources[i].name, name ) )
			return gl3_shader_sources[i].source;
	}
	return NULL;
}

// glShaderSource wants a source whose first line is #version; permutation
// defines therefore have to be spliced in after that line.
static GLuint GL3_CompileShader( GLenum type, const char *source, const char *defines )
{
	const char *nl;
	char *text;
	GLuint shader;
	GLint length;
	GLint status = 0;
	char log[2048];

	shader = qglCreateShader( type );
	if ( !shader )
		return 0;

	if ( defines && defines[0] ) {
		nl = strchr( source, '\n' );
		if ( nl ) {
			size_t head = (size_t)( nl - source ) + 1;
			size_t total = head + strlen( defines ) + strlen( nl + 1 ) + 2;
			text = (char *)ri.Malloc( (int)total );
			Com_Memcpy( text, source, head );
			text[head] = '\0';
			Q_strcat( text, (int)total, defines );
			Q_strcat( text, (int)total, nl + 1 );
		} else {
			text = (char *)ri.Malloc( (int)( strlen( source ) + strlen( defines ) + 2 ) );
			strcpy( text, source );
			Q_strcat( text, (int)( strlen( source ) + strlen( defines ) + 2 ), defines );
		}
	} else {
		text = (char *)ri.Malloc( (int)strlen( source ) + 1 );
		strcpy( text, source );
	}

	length = (GLint)strlen( text );
	qglShaderSource( shader, 1, (const GLchar *const *)&text, &length );
	ri.Free( text );
	qglCompileShader( shader );
	qglGetShaderiv( shader, GL_COMPILE_STATUS, &status );
	if ( !status ) {
		qglGetShaderInfoLog( shader, sizeof( log ), NULL, log );
		ri.Printf( PRINT_WARNING, "GLES3: shader compile failed (%s)%s%s: %s\n",
			type == GL_VERTEX_SHADER ? "vertex" : "fragment",
			defines && defines[0] ? " defines=" : "",
			defines && defines[0] ? defines : "", log );
		qglDeleteShader( shader );
		return 0;
	}

	return shader;
}

static GLuint GL3_LinkProgram( const char *vs_name, const char *fs_name, const char *defines )
{
	const char *vs_src = GL3_ShaderSource( vs_name );
	const char *fs_src = GL3_ShaderSource( fs_name );
	GLuint vs, fs, program;
	GLint status = 0;
	char log[2048];

	if ( !vs_src || !fs_src ) {
		ri.Printf( PRINT_WARNING, "GLES3: missing embedded shader %s/%s\n", vs_name, fs_name );
		return 0;
	}

	vs = GL3_CompileShader( GL_VERTEX_SHADER, vs_src, defines );
	fs = GL3_CompileShader( GL_FRAGMENT_SHADER, fs_src, defines );
	if ( !vs || !fs ) {
		if ( vs ) qglDeleteShader( vs );
		if ( fs ) qglDeleteShader( fs );
		return 0;
	}

	program = qglCreateProgram();
	qglAttachShader( program, vs );
	qglAttachShader( program, fs );
	qglLinkProgram( program );
	qglDeleteShader( vs );
	qglDeleteShader( fs );

	qglGetProgramiv( program, GL_LINK_STATUS, &status );
	if ( !status ) {
		qglGetProgramInfoLog( program, sizeof( log ), NULL, log );
		ri.Printf( PRINT_WARNING, "GLES3: program link failed: %s\n", log );
		qglDeleteProgram( program );
		return 0;
	}

	return program;
}

// ---------------------------------------------------------------------------
// Pipeline definitions -> GL programs
// ---------------------------------------------------------------------------

typedef struct {
	const char *vs;
	const char *fs;
	int			tx;			// texture bundles used (1..3)
	int			env;
	int			color;		// 0 = vertex colors, 1 = identity, 2 = fixed, 3 = entity
	int			cl1;
	int			cl2;
	int			atest;
	int			df;
	int			light;
	int			linear;
	int			fog_only;
	int			color_only;
	int			tex_mode;
	int			color_mode;
} gl3_def_info_t;

static void GL3_ClassifyDef( const Vk_Pipeline_Def *def, gl3_def_info_t *info )
{
	Com_Memset( info, 0, sizeof( *info ) );
	info->vs = "gen_vert.glsl";
	info->fs = "gen_frag.glsl";
	info->tx = 1;
	info->atest = 1;

	switch ( def->shader_type ) {
	case TYPE_SIGNLE_TEXTURE:
		break;
	case TYPE_SIGNLE_TEXTURE_ENV:
		info->env = 1;
		break;
	case TYPE_SIGNLE_TEXTURE_IDENTITY:
		info->color = 1;
		break;
	case TYPE_SIGNLE_TEXTURE_IDENTITY_ENV:
		info->color = 1;
		info->env = 1;
		break;
	case TYPE_SIGNLE_TEXTURE_FIXED_COLOR:
		info->color = 2;
		break;
	case TYPE_SIGNLE_TEXTURE_FIXED_COLOR_ENV:
		info->color = 2;
		info->env = 1;
		break;
	case TYPE_SIGNLE_TEXTURE_ENT_COLOR:
		info->color = 3;
		break;
	case TYPE_SIGNLE_TEXTURE_ENT_COLOR_ENV:
		info->color = 3;
		info->env = 1;
		break;
	case TYPE_SIGNLE_TEXTURE_DF:
		info->color = 1;
		info->df = 1;
		break;
	case TYPE_SIGNLE_TEXTURE_LIGHTING:
	case TYPE_SIGNLE_TEXTURE_LIGHTING_LINEAR:
		info->vs = "light_vert.glsl";
		info->fs = "light_frag.glsl";
		info->light = 1;
		info->linear = ( def->shader_type == TYPE_SIGNLE_TEXTURE_LIGHTING_LINEAR );
		info->atest = 0;
		break;

	case TYPE_MULTI_TEXTURE_ADD2_IDENTITY:
	case TYPE_MULTI_TEXTURE_MUL2_IDENTITY:
		info->tx = 2; info->color = 1; break;
	case TYPE_MULTI_TEXTURE_ADD2_IDENTITY_ENV:
	case TYPE_MULTI_TEXTURE_MUL2_IDENTITY_ENV:
		info->tx = 2; info->color = 1; info->env = 1; break;
	case TYPE_MULTI_TEXTURE_ADD2_FIXED_COLOR:
	case TYPE_MULTI_TEXTURE_MUL2_FIXED_COLOR:
		info->tx = 2; info->color = 2; break;
	case TYPE_MULTI_TEXTURE_ADD2_FIXED_COLOR_ENV:
	case TYPE_MULTI_TEXTURE_MUL2_FIXED_COLOR_ENV:
		info->tx = 2; info->color = 2; info->env = 1; break;
	case TYPE_MULTI_TEXTURE_MUL2:
	case TYPE_MULTI_TEXTURE_ADD2_1_1:
	case TYPE_MULTI_TEXTURE_ADD2:
		info->tx = 2; break;
	case TYPE_MULTI_TEXTURE_MUL2_ENV:
	case TYPE_MULTI_TEXTURE_ADD2_1_1_ENV:
	case TYPE_MULTI_TEXTURE_ADD2_ENV:
		info->tx = 2; info->env = 1; break;
	case TYPE_MULTI_TEXTURE_MUL3:
	case TYPE_MULTI_TEXTURE_ADD3_1_1:
	case TYPE_MULTI_TEXTURE_ADD3:
		info->tx = 3; break;
	case TYPE_MULTI_TEXTURE_MUL3_ENV:
	case TYPE_MULTI_TEXTURE_ADD3_1_1_ENV:
	case TYPE_MULTI_TEXTURE_ADD3_ENV:
		info->tx = 3; info->env = 1; break;

	case TYPE_BLEND2_ADD:
	case TYPE_BLEND2_MUL:
	case TYPE_BLEND2_ALPHA:
	case TYPE_BLEND2_ONE_MINUS_ALPHA:
	case TYPE_BLEND2_MIX_ALPHA:
	case TYPE_BLEND2_MIX_ONE_MINUS_ALPHA:
	case TYPE_BLEND2_DST_COLOR_SRC_ALPHA:
		info->tx = 2; info->cl1 = 1; break;
	case TYPE_BLEND2_ADD_ENV:
	case TYPE_BLEND2_MUL_ENV:
	case TYPE_BLEND2_ALPHA_ENV:
	case TYPE_BLEND2_ONE_MINUS_ALPHA_ENV:
	case TYPE_BLEND2_MIX_ALPHA_ENV:
	case TYPE_BLEND2_MIX_ONE_MINUS_ALPHA_ENV:
	case TYPE_BLEND2_DST_COLOR_SRC_ALPHA_ENV:
		info->tx = 2; info->cl1 = 1; info->env = 1; break;
	case TYPE_BLEND3_ADD:
	case TYPE_BLEND3_MUL:
	case TYPE_BLEND3_ALPHA:
	case TYPE_BLEND3_ONE_MINUS_ALPHA:
	case TYPE_BLEND3_MIX_ALPHA:
	case TYPE_BLEND3_MIX_ONE_MINUS_ALPHA:
	case TYPE_BLEND3_DST_COLOR_SRC_ALPHA:
		info->tx = 3; info->cl1 = 1; info->cl2 = 1; break;
	case TYPE_BLEND3_ADD_ENV:
	case TYPE_BLEND3_MUL_ENV:
	case TYPE_BLEND3_ALPHA_ENV:
	case TYPE_BLEND3_ONE_MINUS_ALPHA_ENV:
	case TYPE_BLEND3_MIX_ALPHA_ENV:
	case TYPE_BLEND3_MIX_ONE_MINUS_ALPHA_ENV:
	case TYPE_BLEND3_DST_COLOR_SRC_ALPHA_ENV:
		info->tx = 3; info->cl1 = 1; info->cl2 = 1; info->env = 1; break;

	case TYPE_COLOR_BLACK:
	case TYPE_COLOR_WHITE:
	case TYPE_COLOR_GREEN:
	case TYPE_COLOR_RED:
		info->vs = "color.vert";
		info->fs = "color.frag";
		info->color_only = 1;
		info->atest = 0;
		if ( def->shader_type == TYPE_COLOR_WHITE ) info->color_mode = 1;
		else if ( def->shader_type == TYPE_COLOR_GREEN ) info->color_mode = 2;
		else if ( def->shader_type == TYPE_COLOR_RED ) info->color_mode = 3;
		break;

	case TYPE_FOG_ONLY:
		info->vs = "fog.vert";
		info->fs = "fog.frag";
		info->fog_only = 1;
		info->atest = 0;
		break;

	case TYPE_DOT:
		// Flares: the Vulkan backend tests visibility by writing a storage
		// buffer from a point sprite. Not ported (see AUDIT.md); map it to the
		// colour program and draw nothing.
		info->vs = "color.vert";
		info->fs = "color.frag";
		info->color_only = 1;
		info->atest = 0;
		break;

	default:
		ri.Printf( PRINT_WARNING, "GLES3: unknown shader type %i, using single texture\n", def->shader_type );
		break;
	}

	// Multitexture mode, mirroring the Vulkan backend's specialization value.
	switch ( def->shader_type ) {
	case TYPE_MULTI_TEXTURE_MUL2_IDENTITY:
	case TYPE_MULTI_TEXTURE_MUL2_IDENTITY_ENV:
	case TYPE_MULTI_TEXTURE_MUL2_FIXED_COLOR:
	case TYPE_MULTI_TEXTURE_MUL2_FIXED_COLOR_ENV:
	case TYPE_MULTI_TEXTURE_MUL2:
	case TYPE_MULTI_TEXTURE_MUL2_ENV:
	case TYPE_MULTI_TEXTURE_MUL3:
	case TYPE_MULTI_TEXTURE_MUL3_ENV:
	case TYPE_BLEND2_MUL:
	case TYPE_BLEND2_MUL_ENV:
	case TYPE_BLEND3_MUL:
	case TYPE_BLEND3_MUL_ENV:
		info->tex_mode = 0;
		break;
	case TYPE_MULTI_TEXTURE_ADD2_IDENTITY:
	case TYPE_MULTI_TEXTURE_ADD2_IDENTITY_ENV:
	case TYPE_MULTI_TEXTURE_ADD2_FIXED_COLOR:
	case TYPE_MULTI_TEXTURE_ADD2_FIXED_COLOR_ENV:
	case TYPE_MULTI_TEXTURE_ADD2_1_1:
	case TYPE_MULTI_TEXTURE_ADD2_1_1_ENV:
	case TYPE_MULTI_TEXTURE_ADD3_1_1:
	case TYPE_MULTI_TEXTURE_ADD3_1_1_ENV:
		info->tex_mode = 1;
		break;
	case TYPE_MULTI_TEXTURE_ADD2:
	case TYPE_MULTI_TEXTURE_ADD2_ENV:
	case TYPE_MULTI_TEXTURE_ADD3:
	case TYPE_MULTI_TEXTURE_ADD3_ENV:
	case TYPE_BLEND2_ADD:
	case TYPE_BLEND2_ADD_ENV:
	case TYPE_BLEND3_ADD:
	case TYPE_BLEND3_ADD_ENV:
		info->tex_mode = 2;
		break;
	case TYPE_BLEND2_ALPHA:
	case TYPE_BLEND2_ALPHA_ENV:
	case TYPE_BLEND3_ALPHA:
	case TYPE_BLEND3_ALPHA_ENV:
		info->tex_mode = 3;
		break;
	case TYPE_BLEND2_ONE_MINUS_ALPHA:
	case TYPE_BLEND2_ONE_MINUS_ALPHA_ENV:
	case TYPE_BLEND3_ONE_MINUS_ALPHA:
	case TYPE_BLEND3_ONE_MINUS_ALPHA_ENV:
		info->tex_mode = 4;
		break;
	case TYPE_BLEND2_MIX_ALPHA:
	case TYPE_BLEND2_MIX_ALPHA_ENV:
	case TYPE_BLEND3_MIX_ALPHA:
	case TYPE_BLEND3_MIX_ALPHA_ENV:
		info->tex_mode = 5;
		break;
	case TYPE_BLEND2_MIX_ONE_MINUS_ALPHA:
	case TYPE_BLEND2_MIX_ONE_MINUS_ALPHA_ENV:
	case TYPE_BLEND3_MIX_ONE_MINUS_ALPHA:
	case TYPE_BLEND3_MIX_ONE_MINUS_ALPHA_ENV:
		info->tex_mode = 6;
		break;
	case TYPE_BLEND2_DST_COLOR_SRC_ALPHA:
	case TYPE_BLEND2_DST_COLOR_SRC_ALPHA_ENV:
	case TYPE_BLEND3_DST_COLOR_SRC_ALPHA:
	case TYPE_BLEND3_DST_COLOR_SRC_ALPHA_ENV:
		info->tex_mode = 7;
		break;
	default:
		break;
	}
}

static gl3_program_t *GL3_GetProgram( const Vk_Pipeline_Def *def )
{
	gl3_def_info_t	info;
	char			defines[512];
	gl3_program_t	*prog;
	GLuint			program;
	int				fog_stage = def->fog_stage ? 1 : 0;
	int				i;

	for ( i = 0; i < gl3_program_count; i++ ) {
		if ( gl3_programs[i].shader_type == (int)def->shader_type &&
		     gl3_programs[i].fog_stage == fog_stage ) {
			return &gl3_programs[i];
		}
	}

	if ( gl3_program_count >= GL3_MAX_PROGRAMS ) {
		ri.Printf( PRINT_WARNING, "GLES3: program cache full\n" );
		return NULL;
	}

	GL3_ClassifyDef( def, &info );

	defines[0] = '\0';
	if ( info.tx >= 2 ) Q_strcat( defines, sizeof( defines ), "#define USE_TX1\n" );
	if ( info.tx >= 3 ) Q_strcat( defines, sizeof( defines ), "#define USE_TX2\n" );
	if ( info.env ) Q_strcat( defines, sizeof( defines ), "#define USE_ENV\n" );
	if ( fog_stage ) Q_strcat( defines, sizeof( defines ), "#define USE_FOG\n" );
	if ( info.color == 1 ) Q_strcat( defines, sizeof( defines ), "#define USE_CLX_IDENT\n" );
	if ( info.color == 2 ) Q_strcat( defines, sizeof( defines ), "#define USE_FIXED_COLOR\n" );
	if ( info.color == 3 ) {
		// The vertex stage has no entity-colour variant: it uses the fixed
		// colour path, the fragment stage reads ent_color0 from the UBO.
		Q_strcat( defines, sizeof( defines ), "#define USE_FIXED_COLOR\n" );
		Q_strcat( defines, sizeof( defines ), "#define USE_ENT_COLOR\n" );
	}
	if ( info.cl1 ) Q_strcat( defines, sizeof( defines ), "#define USE_CL1\n" );
	if ( info.cl2 ) Q_strcat( defines, sizeof( defines ), "#define USE_CL2\n" );
	if ( info.atest ) Q_strcat( defines, sizeof( defines ), "#define USE_ATEST\n" );
	if ( info.df ) Q_strcat( defines, sizeof( defines ), "#define USE_DF\n" );
	if ( info.linear ) Q_strcat( defines, sizeof( defines ), "#define USE_LINE\n" );

	program = GL3_LinkProgram( info.vs, info.fs, defines );
	if ( !program ) {
		ri.Printf( PRINT_WARNING, "GLES3: could not build program for shader type %i\n", def->shader_type );
		return NULL;
	}

	prog = &gl3_programs[gl3_program_count++];
	Com_Memset( prog, 0, sizeof( *prog ) );
	prog->shader_type = (int)def->shader_type;
	prog->fog_stage = fog_stage;
	prog->program = program;

	prog->loc_mvp = qglGetUniformLocation( program, "mvp" );
	prog->loc_texture0 = qglGetUniformLocation( program, "texture0" );
	prog->loc_texture1 = qglGetUniformLocation( program, "texture1" );
	prog->loc_texture2 = qglGetUniformLocation( program, "texture2" );
	prog->loc_fog_texture = qglGetUniformLocation( program, "fog_texture" );
	prog->loc_alpha_test_func = qglGetUniformLocation( program, "alpha_test_func" );
	prog->loc_alpha_test_value = qglGetUniformLocation( program, "alpha_test_value" );
	prog->loc_depth_fragment = qglGetUniformLocation( program, "depth_fragment" );
	prog->loc_alpha_to_coverage = qglGetUniformLocation( program, "alpha_to_coverage" );
	prog->loc_color_mode = qglGetUniformLocation( program, "color_mode" );
	prog->loc_abs_light = qglGetUniformLocation( program, "abs_light" );
	prog->loc_tex_mode = qglGetUniformLocation( program, "tex_mode" );
	prog->loc_discard_mode = qglGetUniformLocation( program, "discard_mode" );
	prog->loc_identity_color = qglGetUniformLocation( program, "identity_color" );
	prog->loc_identity_alpha = qglGetUniformLocation( program, "identity_alpha" );
	prog->loc_acff = qglGetUniformLocation( program, "acff" );

	// Texture units are fixed by layout(binding=N) in the GLSL, but setting the
	// sampler uniforms explicitly is both harmless and robust across drivers.
	if ( prog->loc_texture0 >= 0 ) qglUniform1i( prog->loc_texture0, 0 );
	if ( prog->loc_texture1 >= 0 ) qglUniform1i( prog->loc_texture1, 1 );
	if ( prog->loc_texture2 >= 0 ) qglUniform1i( prog->loc_texture2, 2 );
	if ( prog->loc_fog_texture >= 0 ) qglUniform1i( prog->loc_fog_texture, 3 );

	// Defaults the Vulkan specialization constants used to bake in.
	if ( prog->loc_alpha_test_func >= 0 ) qglUniform1i( prog->loc_alpha_test_func, 0 );
	if ( prog->loc_alpha_test_value >= 0 ) qglUniform1f( prog->loc_alpha_test_value, 0.0f );
	if ( prog->loc_depth_fragment >= 0 ) qglUniform1f( prog->loc_depth_fragment, 0.85f );
	if ( prog->loc_alpha_to_coverage >= 0 ) qglUniform1i( prog->loc_alpha_to_coverage, 0 );
	if ( prog->loc_color_mode >= 0 ) qglUniform1i( prog->loc_color_mode, info.color_mode );
	if ( prog->loc_abs_light >= 0 ) qglUniform1i( prog->loc_abs_light, 0 );
	if ( prog->loc_tex_mode >= 0 ) qglUniform1i( prog->loc_tex_mode, info.tex_mode );
	if ( prog->loc_discard_mode >= 0 ) qglUniform1i( prog->loc_discard_mode, 0 );
	if ( prog->loc_identity_color >= 0 ) qglUniform1f( prog->loc_identity_color, 1.0f );
	if ( prog->loc_identity_alpha >= 0 ) qglUniform1f( prog->loc_identity_alpha, 1.0f );
	if ( prog->loc_acff >= 0 ) qglUniform1i( prog->loc_acff, 0 );

	return prog;
}

// ---------------------------------------------------------------------------
// Depth/state mapping (mirrors create_pipeline's state setup)
// ---------------------------------------------------------------------------

static GLenum GL3_BlendFactor( unsigned int bits )
{
	switch ( bits ) {
	case GLS_SRCBLEND_ZERO:
	case GLS_DSTBLEND_ZERO:
		return GL_ZERO;
	case GLS_SRCBLEND_ONE:
	case GLS_DSTBLEND_ONE:
		return GL_ONE;
	case GLS_SRCBLEND_DST_COLOR:
	case GLS_DSTBLEND_SRC_COLOR:
		return GL_DST_COLOR;
	case GLS_SRCBLEND_ONE_MINUS_DST_COLOR:
	case GLS_DSTBLEND_ONE_MINUS_SRC_COLOR:
		return GL_ONE_MINUS_DST_COLOR;
	case GLS_SRCBLEND_SRC_ALPHA:
	case GLS_DSTBLEND_SRC_ALPHA:
		return GL_SRC_ALPHA;
	case GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA:
	case GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA:
		return GL_ONE_MINUS_SRC_ALPHA;
	case GLS_SRCBLEND_DST_ALPHA:
	case GLS_DSTBLEND_DST_ALPHA:
		return GL_DST_ALPHA;
	case GLS_SRCBLEND_ONE_MINUS_DST_ALPHA:
	case GLS_DSTBLEND_ONE_MINUS_DST_ALPHA:
		return GL_ONE_MINUS_DST_ALPHA;
	case GLS_SRCBLEND_ALPHA_SATURATE:
		return GL_SRC_ALPHA_SATURATE;
	default:
		return GL_ONE;
	}
}

static void GL3_SetState( const Vk_Pipeline_Def *def )
{
	unsigned int state_bits = def->state_bits;

	if ( state_bits & GLS_DEPTHTEST_DISABLE )
		qglDisable( GL_DEPTH_TEST );
	else
		qglEnable( GL_DEPTH_TEST );

	qglDepthMask( ( state_bits & GLS_DEPTHMASK_TRUE ) ? GL_TRUE : GL_FALSE );
	qglDepthFunc( ( state_bits & GLS_DEPTHFUNC_EQUAL ) ? GL_EQUAL : GL_GEQUAL );

	if ( def->depth_clear ) {
		qglEnable( GL_DEPTH_TEST );
		qglDepthMask( GL_TRUE );
		qglDepthFunc( GL_ALWAYS );
	}

	if ( state_bits & ( GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS ) ) {
		qglEnable( GL_BLEND );
		qglBlendFunc( GL3_BlendFactor( state_bits & GLS_SRCBLEND_BITS ),
		              GL3_BlendFactor( state_bits & GLS_DSTBLEND_BITS ) );
	} else {
		qglDisable( GL_BLEND );
	}

	switch ( def->face_culling ) {
	case CT_FRONT_SIDED:
		qglEnable( GL_CULL_FACE );
		qglCullFace( def->mirror ? GL_FRONT : GL_BACK );
		break;
	case CT_BACK_SIDED:
		qglEnable( GL_CULL_FACE );
		qglCullFace( def->mirror ? GL_BACK : GL_FRONT );
		break;
	default:
		qglDisable( GL_CULL_FACE );
		break;
	}

	if ( def->polygon_offset ) {
		qglEnable( GL_POLYGON_OFFSET_FILL );
		// Reversed depth: a decal must end up closer, i.e. at a larger depth.
		qglPolygonOffset( -r_offsetFactor->value, -r_offsetUnits->value );
	} else {
		qglDisable( GL_POLYGON_OFFSET_FILL );
	}

	if ( def->line_width > 1 ) {
		// GLES core only guarantees width 1; drivers may clamp silently. Depth
		// prepass is off by default for exactly this reason.
		qglLineWidth( (GLfloat)def->line_width );
	} else {
		qglLineWidth( 1.0f );
	}

	// Stencil shadows (r_shadows 2), same phases as the Vulkan pipelines.
	if ( def->shadow_phase != SHADOW_DISABLED ) {
		qglEnable( GL_STENCIL_TEST );
		qglStencilMask( 0xFF );
		if ( def->shadow_phase == SHADOW_EDGES ) {
			qglStencilFunc( GL_ALWAYS, 0, 0xFF );
			qglStencilOp( GL_KEEP, GL_KEEP,
				( def->face_culling == CT_FRONT_SIDED ) ? GL_INCR : GL_DECR );
			qglColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
		} else {
			qglStencilFunc( GL_NOTEQUAL, 0, 0xFF );
			qglStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
			qglColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
		}
	} else {
		qglDisable( GL_STENCIL_TEST );
		qglColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	}

	if ( def->shader_type == TYPE_SIGNLE_TEXTURE_DF || def->depth_clear )
		qglColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

	qglFrontFace( GL_CW );	// Q3 default vertex order
}

// ---------------------------------------------------------------------------
// Viewport / scissor (same rectangles the Vulkan backend computes)
// ---------------------------------------------------------------------------

static void GL3_ViewportRect( int *x, int *y, int *w, int *h )
{
	if ( backEnd.projection2D ) {
		*x = 0;
		*y = 0;
		*w = gl3_width;
		*h = gl3_height;
	} else {
		// GL viewport origin is bottom-left; the Vulkan backend's Y expression
		// is already that value (renderHeight - top - height).
		*x = (int)( backEnd.viewParms.viewportX * vk.renderScaleX );
		*y = (int)( vk.renderHeight - ( backEnd.viewParms.viewportY + backEnd.viewParms.viewportHeight ) * vk.renderScaleY );
		*w = (int)( backEnd.viewParms.viewportWidth * vk.renderScaleX );
		*h = (int)( backEnd.viewParms.viewportHeight * vk.renderScaleY );
	}

	if ( *x < 0 ) *x = 0;
	if ( *y < 0 ) *y = 0;
	if ( *x + *w > gl3_width ) *w = gl3_width - *x;
	if ( *y + *h > gl3_height ) *h = gl3_height - *y;
	if ( *w < 0 ) *w = 0;
	if ( *h < 0 ) *h = 0;
}

static void GL3_ScissorRect( int *x, int *y, int *w, int *h )
{
	if ( backEnd.viewParms.portalView != PV_NONE ) {
		*x = backEnd.viewParms.scissorX;
		*y = gl3_height - backEnd.viewParms.scissorY - backEnd.viewParms.scissorHeight;
		*w = backEnd.viewParms.scissorWidth;
		*h = backEnd.viewParms.scissorHeight;

		if ( *x < 0 ) *x = 0;
		if ( *y < 0 ) *y = 0;
		if ( *x + *w > gl3_width ) *w = gl3_width - *x;
		if ( *y + *h > gl3_height ) *h = gl3_height - *y;
		if ( *w < 0 ) *w = 0;
		if ( *h < 0 ) *h = 0;
	} else {
		GL3_ViewportRect( x, y, w, h );
	}
}

static void GL3_UpdateViewport( Vk_Depth_Range depth_range )
{
	int x, y, w, h;

	GL3_ViewportRect( &x, &y, &w, &h );
	qglViewport( x, y, w, h );

	GL3_ScissorRect( &x, &y, &w, &h );
	qglScissor( x, y, w, h );
	qglEnable( GL_SCISSOR_TEST );

	switch ( depth_range ) {
	case DEPTH_RANGE_ZERO:
#ifdef USE_REVERSED_DEPTH
		qglDepthRangef( 1.0f, 1.0f );
#else
		qglDepthRangef( 0.0f, 0.0f );
#endif
		break;
	case DEPTH_RANGE_ONE:
#ifdef USE_REVERSED_DEPTH
		qglDepthRangef( 0.0f, 0.0f );
#else
		qglDepthRangef( 1.0f, 1.0f );
#endif
		break;
	case DEPTH_RANGE_WEAPON:
#ifdef USE_REVERSED_DEPTH
		qglDepthRangef( 0.6f, 1.0f );
#else
		qglDepthRangef( 0.0f, 0.3f );
#endif
		break;
	default:
		qglDepthRangef( 0.0f, 1.0f );
		break;
	}
}

// ---------------------------------------------------------------------------
// MVP (GL conventions: no Vulkan Y flip, 2D flips Y)
// ---------------------------------------------------------------------------

static void GL3_DefaultMVP( float *mvp )
{
	if ( backEnd.projection2D ) {
		float w = (float)gl3_width;
		float h = (float)gl3_height;

		mvp[0]  =  2.0f / w; mvp[1]  =  0.0f;      mvp[2]  = 0.0f; mvp[3]  = 0.0f;
		mvp[4]  =  0.0f;     mvp[5]  = -2.0f / h;   mvp[6]  = 0.0f; mvp[7]  = 0.0f;
		mvp[8]  =  0.0f;     mvp[9]  =  0.0f;       mvp[10] = 0.0f; mvp[11] = 0.0f;
		mvp[12] = -1.0f;     mvp[13] =  1.0f;       mvp[14] = 1.0f; mvp[15] = 1.0f;
	} else {
		// vk_world.modelview_transform is what the shared front end refreshes
		// before each draw (world, entity or shadow pass); using it keeps the
		// GL path identical to the Vulkan one, minus its Y flip.
		myGlMultMatrix( vk_world.modelview_transform,
		                backEnd.viewParms.projectionMatrix, mvp );
	}
}

// ---------------------------------------------------------------------------
// GL objects
// ---------------------------------------------------------------------------

static void GL3_CreateTarget( GLuint *fbo, GLuint *color, GLuint *depth )
{
	qglGenTextures( 1, color );
	qglBindTexture( GL_TEXTURE_2D, *color );
	qglTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, gl3_width, gl3_height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, NULL );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	qglGenRenderbuffers( 1, depth );
	qglBindRenderbuffer( GL_RENDERBUFFER, *depth );
	qglRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, gl3_width, gl3_height );

	qglGenFramebuffers( 1, fbo );
	qglBindFramebuffer( GL_FRAMEBUFFER, *fbo );
	qglFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *color, 0 );
	qglFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *depth );

	if ( qglCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
		ri.Printf( PRINT_WARNING, "GLES3: framebuffer incomplete\n" );

	qglBindFramebuffer( GL_FRAMEBUFFER, 0 );
	qglBindRenderbuffer( GL_RENDERBUFFER, 0 );
	qglBindTexture( GL_TEXTURE_2D, 0 );
}

static void GL3_DestroyTarget( GLuint *fbo, GLuint *color, GLuint *depth )
{
	if ( *fbo ) { qglDeleteFramebuffers( 1, fbo ); *fbo = 0; }
	if ( *color ) { qglDeleteTextures( 1, color ); *color = 0; }
	if ( *depth ) { qglDeleteRenderbuffers( 1, depth ); *depth = 0; }
}

static void GL3_CreateTargets( void )
{
	if ( gl3_main_fbo )
		return;

	GL3_CreateTarget( &gl3_main_fbo, &gl3_main_color, &gl3_main_depth );
	GL3_CreateTarget( &gl3_screenmap_fbo, &gl3_screenmap_color, &gl3_screenmap_depth );

	// The screenMap pass writes the mirror image; the front end binds it by
	// descriptor, the same convention as an image_t.
	vk.screenMap.color_descriptor = GL3_DescriptorFromTexture( gl3_screenmap_color );
	vk.screenMap.color_image_view = (VkImageView)(uintptr_t)gl3_screenmap_color;
}

static void GL3_CreateGammaProgram( void )
{
	GLuint vs, fs;
	const char *vs_src = GL3_ShaderSource( "gamma.vert" );
	const char *fs_src = GL3_ShaderSource( "gamma.frag" );
	GLint status = 0;
	char log[2048];

	if ( !vs_src || !fs_src )
		return;

	vs = GL3_CompileShader( GL_VERTEX_SHADER, vs_src, NULL );
	fs = GL3_CompileShader( GL_FRAGMENT_SHADER, fs_src, NULL );
	if ( !vs || !fs ) {
		if ( vs ) qglDeleteShader( vs );
		if ( fs ) qglDeleteShader( fs );
		return;
	}

	gl3_gamma_program = qglCreateProgram();
	qglAttachShader( gl3_gamma_program, vs );
	qglAttachShader( gl3_gamma_program, fs );
	qglLinkProgram( gl3_gamma_program );
	qglDeleteShader( vs );
	qglDeleteShader( fs );
	qglGetProgramiv( gl3_gamma_program, GL_LINK_STATUS, &status );
	if ( !status ) {
		qglGetProgramInfoLog( gl3_gamma_program, sizeof( log ), NULL, log );
		ri.Printf( PRINT_WARNING, "GLES3: gamma program link failed: %s\n", log );
		qglDeleteProgram( gl3_gamma_program );
		gl3_gamma_program = 0;
		return;
	}

	gl3_gamma_loc[0] = qglGetUniformLocation( gl3_gamma_program, "texture0" );
	gl3_gamma_loc[1] = qglGetUniformLocation( gl3_gamma_program, "gamma" );
	gl3_gamma_loc[2] = qglGetUniformLocation( gl3_gamma_program, "obScale" );
	gl3_gamma_loc[3] = qglGetUniformLocation( gl3_gamma_program, "greyscale" );
	gl3_gamma_loc[4] = qglGetUniformLocation( gl3_gamma_program, "ditherMode" );
	gl3_gamma_loc[5] = qglGetUniformLocation( gl3_gamma_program, "depth_r" );
	gl3_gamma_loc[6] = qglGetUniformLocation( gl3_gamma_program, "depth_g" );
	gl3_gamma_loc[7] = qglGetUniformLocation( gl3_gamma_program, "depth_b" );
}

// ---------------------------------------------------------------------------
// Backend entry points
// ---------------------------------------------------------------------------

void vk_initialize( void )
{
	if ( gl3_initialized )
		return;

	if ( !gl3_uniform_stage ) {
		gl3_uniform_stage = (byte *)ri.Malloc( GL3_UNIFORM_STAGE_SIZE );
		Com_Memset( gl3_uniform_stage, 0, GL3_UNIFORM_STAGE_SIZE );
	}

	// First init created the window in tr_init's seam; a vid_restart
	// (KEEP_WINDOW) re-enters here with the old window still alive, so re-apply
	// r_mode / r_fullscreen before sizing the targets.
	GL3_PlatformResync( &glConfig );

	gl3_width = glConfig.vidWidth;
	gl3_height = glConfig.vidHeight;

	vk.renderWidth = gl3_width;
	vk.renderHeight = gl3_height;
	vk.renderScaleX = 1.0f;
	vk.renderScaleY = 1.0f;
	vk.fboActive = qtrue;	// renders to gl3_main_fbo and composites at frame end
	vk.offscreenRender = qtrue;
	vk.active = qtrue;
	vk.renderPassIndex = RENDER_PASS_MAIN;
	vk.uniform_alignment = 256;
	vk.uniform_item_size = (uint32_t)PAD( sizeof( vkUniform_t ), 16 );
	vk.geometry_buffer_size = GL3_UNIFORM_STAGE_SIZE;
	vk.maxTextureClamp = 0;
	vk.extraPicmip = 0;
	vk.samplerAnisotropy = qfalse;
	vk.maxAnisotropy = 0;
	// Stay on the classic (multipass) dynamic-light path and the <=2 texture
	// unit shader combinations; the 3-stage "mtEnv3" combinations need the
	// 8-descriptor paths this backend does not implement.
	vk.maxBoundDescriptorSets = 0;
	if ( r_dlightMode->integer != 0 ) {
		ri.Printf( PRINT_ALL, "GLES3: r_dlightMode %i needs the per-pixel light pass; using the classic multipass path\n",
			r_dlightMode->integer );
		ri.Cvar_Set( "r_dlightMode", "0" );
	}

	if ( r_smp->integer ) {
		// Documented decision: this backend is single-threaded. GLES contexts
		// are thread-affine and neither VAOs nor FBOs are shared between
		// contexts, so the engine's r_smp render-thread design (one GL context,
		// two threads) cannot be made correct without a second shared context
		// and a VAO/FBO migration scheme. The default configuration ships
		// r_smp 0, so forcing it here changes nothing for normal use and keeps
		// the renderer honest instead of racing the context.
		ri.Printf( PRINT_ALL, "GLES3: r_smp is not supported by this backend (single GL context); forcing r_smp 0\n" );
		ri.Cvar_Set( "r_smp", "0" );
	}

	gl3_context_lost = qfalse;
	gl3_gl_errors = 0;
	gl3_first_frame_checked = 0;

	vk.cmd = &gl3_cmd;
	Com_Memset( &gl3_cmd, 0, sizeof( gl3_cmd ) );
	gl3_cmd.vertex_buffer_ptr = gl3_uniform_stage;
	gl3_cmd.vertex_buffer = (VkBuffer)(uintptr_t)1;	// non-null sentinel
	gl3_cmd.uniform_descriptor = (VkDescriptorSet)(uintptr_t)1;

	vk.pipelines_count = 0;
	vk.pipelines_world_base = 0;

	gls.captureWidth = gl3_width;
	gls.captureHeight = gl3_height;

	qglGenVertexArrays( 1, &gl3_vao );
	qglBindVertexArray( gl3_vao );
	qglGenVertexArrays( 1, &gl3_gamma_vao );	// attribute-less: gamma.vert uses gl_VertexID
	qglGenBuffers( 1, &gl3_vbo );
	qglGenBuffers( 1, &gl3_ibo );
	qglGenBuffers( 1, &gl3_ubo );
	qglBindVertexArray( 0 );

	GL3_CreateTargets();
	GL3_CHECK( "create targets" );
	GL3_CreateGammaProgram();
	GL3_CHECK( "gamma program" );

	// Default GL state the renderer assumes.
	qglDisable( GL_DEPTH_TEST );
	qglDepthMask( GL_FALSE );
	qglDisable( GL_BLEND );
	qglDisable( GL_CULL_FACE );
	qglDisable( GL_SCISSOR_TEST );
	qglDisable( GL_STENCIL_TEST );
	qglColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	qglFrontFace( GL_CW );

	GL3_DefaultMVP( gl3_mvp_default );
	Com_Memcpy( gl3_mvp, gl3_mvp_default, sizeof( gl3_mvp ) );

	gl3_initialized = qtrue;
	gl3_backend_ok = qtrue;

	if ( !gl3_selftest_registered ) {
		gl3_selftest_registered = qtrue;
		ri.Cmd_AddCommand( "gles3_selftest", GL3_SelfTest_f );
	}

	GL3_LogGLInfo();
	ri.Printf( PRINT_ALL, "GLES3: backend initialized (%dx%d, %s, programs=%d)\n",
		gl3_width, gl3_height, gl3_gamma_program ? "gamma pass on" : "no gamma pass",
		gl3_program_count );
}

void vk_init_descriptors( void )
{
}

void vk_release_resources( void )
{
	vk.pipelines_count = 0;
}

void vk_shutdown( refShutdownCode_t code )
{
	int i;

	(void)code;

	for ( i = 0; i < gl3_program_count; i++ ) {
		if ( gl3_programs[i].program )
			qglDeleteProgram( gl3_programs[i].program );
	}
	gl3_program_count = 0;

	GL3_DestroyTarget( &gl3_main_fbo, &gl3_main_color, &gl3_main_depth );
	GL3_DestroyTarget( &gl3_screenmap_fbo, &gl3_screenmap_color, &gl3_screenmap_depth );

	if ( gl3_gamma_program ) { qglDeleteProgram( gl3_gamma_program ); gl3_gamma_program = 0; }
	if ( gl3_vao ) { qglDeleteVertexArrays( 1, &gl3_vao ); gl3_vao = 0; }
	if ( gl3_vbo ) { qglDeleteBuffers( 1, &gl3_vbo ); gl3_vbo = 0; }
	if ( gl3_ibo ) { qglDeleteBuffers( 1, &gl3_ibo ); gl3_ibo = 0; }
	if ( gl3_ubo ) { qglDeleteBuffers( 1, &gl3_ubo ); gl3_ubo = 0; }

	if ( gl3_uniform_stage ) {
		ri.Free( gl3_uniform_stage );
		gl3_uniform_stage = NULL;
	}

	vk.active = qfalse;
	vk.fboActive = qfalse;
	gl3_initialized = qfalse;
	gl3_backend_ok = qfalse;

	// REF_KEEP_WINDOW (a vid_restart) must keep the SDL window and the GL
	// context: the engine intentionally does not recreate them, and the next
	// vk_initialize() only re-syncs r_mode/r_fullscreen (GL3_PlatformResync).
	// Only a real unload tears the platform down.
	if ( code == REF_UNLOAD_DLL )
		GL3_PlatformShutdown( qtrue );
}

void vk_wait_idle( void )
{
	qglFinish();
}

void vk_queue_wait_idle( void )
{
	qglFinish();
}

void vk_reset_registration_stats( void )
{
	vk.reg_img_count = 0;
	vk.reg_img_bytes = 0;
	vk.reg_img_bytes_full = 0;
	vk.reg_img_reduced = 0;
	vk.reg_img_biggest = 0;
	vk.reg_img_biggest_name[0] = '\0';
}

int vk_registration_report( char *out, int outSize )
{
	if ( !out || outSize <= 0 )
		return 0;
	out[0] = '\0';
	if ( vk.reg_img_count ) {
		Com_sprintf( out, outSize, "textures: %u uploaded, %u bytes\n",
			vk.reg_img_count, vk.reg_img_bytes );
		return (int)strlen( out );
	}
	return 0;
}

void vk_create_image( image_t *image, int width, int height, int mip_levels )
{
	GLuint tex = 0;

	qglGenTextures( 1, &tex );
	qglBindTexture( GL_TEXTURE_2D, tex );

	switch ( image->wrapClampMode ) {
	case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER:
	case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		break;
	case VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT:
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT );
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT );
		break;
	default:
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		break;
	}

	// vk.samplers holds the GL filter enums the texture menu selected. A
	// non-mipmapped image must not keep a mipmap min filter, or the texture is
	// incomplete and samples black.
	{
		int mag = vk.samplers.filter_max ? vk.samplers.filter_max : GL_LINEAR;
		int min = vk.samplers.filter_min ? vk.samplers.filter_min : GL_LINEAR;

		if ( !( image->flags & IMGFLAG_MIPMAP ) ) {
			if ( min != GL_NEAREST && min != GL_LINEAR )
				min = ( mag == GL_NEAREST ) ? GL_NEAREST : GL_LINEAR;
		}
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min );
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag );
	}

	// The upload path provides the complete mip chain at once, so the storage is
	// allocated there; only the size is recorded here.
	image->uploadWidth = width;
	image->uploadHeight = height;
	image->handle = (VkImage)(uintptr_t)tex;
	image->descriptor = GL3_DescriptorFromTexture( tex );
	(void)mip_levels;

	qglBindTexture( GL_TEXTURE_2D, 0 );
	gl3_texture_count++;
}

void vk_upload_image_data( image_t *image, int x, int y, int width, int height,
	int mipmaps, byte *pixels, int size, qboolean update )
{
	GLuint tex = (GLuint)(uintptr_t)image->handle;
	int level = 0;
	int w = width, h = height;
	byte *src = pixels;

	if ( !tex || !pixels )
		return;

	qglBindTexture( GL_TEXTURE_2D, tex );

	if ( update ) {
		// Lightmap sub-uploads and cinematic frames write into an existing
		// level.
		qglPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
		qglTexSubImage2D( GL_TEXTURE_2D, 0, x, y, width, height,
			GL_RGBA, GL_UNSIGNED_BYTE, pixels );
		qglBindTexture( GL_TEXTURE_2D, 0 );
		return;
	}

	qglPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
	while ( level < mipmaps && ( w > 0 || h > 0 ) ) {
		int mw = w > 0 ? w : 1;
		int mh = h > 0 ? h : 1;
		int mip_size = mw * mh * 4;

		if ( ( src - pixels ) + mip_size > size )
			break;

		qglTexImage2D( GL_TEXTURE_2D, level, GL_RGBA8, mw, mh, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, src );

		src += mip_size;
		w /= 2;
		h /= 2;
		level++;
	}

	if ( mipmaps > 1 ) {
		// The chain is complete; keep the mip-filter the menu asked for.
		qglTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			vk.samplers.filter_min ? vk.samplers.filter_min : GL_LINEAR_MIPMAP_LINEAR );
	}

	qglBindTexture( GL_TEXTURE_2D, 0 );
}

void vk_update_descriptor_set( image_t *image, qboolean mipmap )
{
	(void)mipmap;
	image->descriptor = GL3_DescriptorFromTexture( (GLuint)(uintptr_t)image->handle );
}

void vk_destroy_image_resources( VkImage *image, VkImageView *imageView )
{
	GLuint tex;

	if ( image && *image ) {
		tex = (GLuint)(uintptr_t)*image;
		qglDeleteTextures( 1, &tex );
		*image = 0;
		if ( gl3_texture_count > 0 )
			gl3_texture_count--;
	}
	if ( imageView )
		*imageView = 0;
}

void vk_update_attachment_descriptors( void )
{
}

void vk_destroy_samplers( void )
{
}

uint32_t vk_find_pipeline_ext( uint32_t base, const Vk_Pipeline_Def *def, qboolean use )
{
	uint32_t index;

	(void)use;	// GL programs are created on first bind

	for ( index = base; index < vk.pipelines_count; index++ ) {
		if ( memcmp( &vk.pipelines[index].def, def, sizeof( *def ) ) == 0 )
			return index;
	}

	if ( vk.pipelines_count >= MAX_VK_PIPELINES ) {
		ri.Error( ERR_DROP, "GLES3: MAX_VK_PIPELINES reached" );
		return 0;
	}

	index = vk.pipelines_count++;
	Com_Memcpy( &vk.pipelines[index].def, def, sizeof( *def ) );
	return index;
}

void vk_get_pipeline_def( uint32_t pipeline, Vk_Pipeline_Def *def )
{
	if ( pipeline >= vk.pipelines_count )
		Com_Memset( def, 0, sizeof( *def ) );
	else
		Com_Memcpy( def, &vk.pipelines[pipeline].def, sizeof( *def ) );
}

void vk_prewarm_pipelines( void )
{
	// Compilation is per draw (first use), not upfront: creating every program
	// permutation the map might need is the pipeline-prewarm analogue and is
	// worth doing once the backend is verified.
}

void vk_save_pipeline_cache( void )
{
}

void vk_create_post_process_pipeline( int program_index, uint32_t width, uint32_t height )
{
	(void)program_index; (void)width; (void)height;
}

void vk_update_post_process_pipelines( void )
{
}

void vk_create_pipelines( void )
{
}

void vk_clear_color( const vec4_t color )
{
	if ( !gl3_frame_active )
		return;
	Vector4Copy( color, gl3_clear_color );
	qglDisable( GL_SCISSOR_TEST );
	qglClearColor( color[0], color[1], color[2], color[3] );
	qglClear( GL_COLOR_BUFFER_BIT );
}

void vk_clear_depth( qboolean clear_stencil )
{
	if ( !gl3_frame_active )
		return;

	// Reversed depth: 0 is the far plane.
	qglClearDepthf( 0.0f );
	qglClearStencil( 0 );
	qglDisable( GL_SCISSOR_TEST );
	qglDepthMask( GL_TRUE );
	if ( clear_stencil && glConfig.stencilBits > 0 ) {
		qglStencilMask( 0xFF );
		qglClear( GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	} else {
		qglClear( GL_DEPTH_BUFFER_BIT );
	}
	qglDepthMask( GL_FALSE );
}

// Same command-list peek as the Vulkan backend: the front end flags the first
// drawSurfs command with needScreenMap and the last with switchRenderPass.
// ScreenMap pass ordering (the reason for the frame_count guard): the front end
// emits, per frame, [RC_DRAW_BUFFER, RC_DRAW_SURFS] per view. A mirror-shaped
// view has needScreenMap on its first drawSurfs command and switchRenderPass on
// the last. vk_begin_frame() peeks the first command, binds the screenmap
// target when that flag is set, and later RB_DrawSurfs calls
// vk_end_render_pass + vk_begin_main_render_pass on switchRenderPass. Every
// subsequent RC_DRAW_BUFFER in the same frame must NOT re-open or clear a
// target, which is what the frame_count increment/early-return (and the
// matching reset in vk_end_frame) enforces -- the same rule as the Vulkan
// backend. Without it, a portal's second view would wipe the first view's
// output with a fresh clear.
static qboolean GL3_FindScreenmapDrawsurfs( void )
{
	const void *curCmd = &backEndData->commands.cmds;

	for ( ;; ) {
		curCmd = PADP( curCmd, sizeof( void * ) );
		switch ( *(const int *)curCmd ) {
		case RC_DRAW_BUFFER:
			curCmd = (const void *)( (const drawBufferCommand_t *)curCmd + 1 );
			break;
		case RC_DRAW_SURFS:
			return ( (const drawSurfsCommand_t *)curCmd )->refdef.needScreenMap;
		default:
			return qfalse;
		}
	}
}

static void GL3_BeginTarget( GLuint fbo, qboolean clear )
{
	qglBindFramebuffer( GL_FRAMEBUFFER, fbo );
	qglViewport( 0, 0, gl3_width, gl3_height );
	qglScissor( 0, 0, gl3_width, gl3_height );
	qglEnable( GL_SCISSOR_TEST );

	if ( clear ) {
		qglClearColor( gl3_clear_color[0], gl3_clear_color[1], gl3_clear_color[2], gl3_clear_color[3] );
		qglClearDepthf( 0.0f );	// reversed depth
		qglClearStencil( 0 );
		qglDepthMask( GL_TRUE );
		qglStencilMask( 0xFF );
		qglClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
		qglDepthMask( GL_FALSE );
	}
}

void vk_begin_main_render_pass( void )
{
	vk.renderPassIndex = RENDER_PASS_MAIN;
	GL3_BeginTarget( gl3_main_fbo, qtrue );
	qglUseProgram( 0 );
	vk.cmd->last_pipeline = 0;
}

void vk_end_render_pass( void )
{
}

void vk_begin_frame( void )
{
	// One frame may issue several draw-buffer commands (portals, mirrors,
	// stereo); only the first one opens the frame, exactly like the Vulkan
	// backend's vk.frame_count guard.
	if ( vk.frame_count++ )
		return;

	gl3_frame_active = qtrue;
	vk.renderPassIndex = RENDER_PASS_MAIN;

	gl3_cmd.vertex_buffer_offset = 0;
	gl3_cmd.uniform_read_offset = 0;
	gl3_cmd.num_indexes = 0;
	gl3_cmd.depth_range = DEPTH_RANGE_COUNT;
	gl3_cmd.last_pipeline = 0;
	Com_Memset( &gl3_cmd.descriptor_set, 0, sizeof( gl3_cmd.descriptor_set ) );
	Com_Memset( &gl3_cmd.scissor_rect, 0, sizeof( gl3_cmd.scissor_rect ) );

	gl3_clear_color[0] = 0.0f;
	gl3_clear_color[1] = 0.0f;
	gl3_clear_color[2] = 0.0f;
	gl3_clear_color[3] = 1.0f;

	Com_Memcpy( gl3_mvp, gl3_mvp_default, sizeof( gl3_mvp ) );

	gl3_screenmap_pending = GL3_FindScreenmapDrawsurfs();

	qglBindVertexArray( gl3_vao );
	qglBindBuffer( GL_ARRAY_BUFFER, gl3_vbo );
	qglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gl3_ibo );
	qglBindBuffer( GL_UNIFORM_BUFFER, gl3_ubo );
	Com_Memset( gl3_uniform_stage, 0, GL3_UNIFORM_STAGE_SIZE );

	if ( gl3_screenmap_pending ) {
		// Mirror/portal pass renders first into its own target; the front end
		// switches to the main target when it reaches the flagged drawSurfs.
		vk.renderPassIndex = RENDER_PASS_SCREENMAP;
		GL3_BeginTarget( gl3_screenmap_fbo, qtrue );
	} else {
		vk_begin_main_render_pass();
	}
}

void vk_end_frame( void )
{
	if ( vk.frame_count == 0 )
		return;
	vk.frame_count = 0;

	qglBindFramebuffer( GL_FRAMEBUFFER, 0 );
	gl3_frame_active = qfalse;
}

void vk_present_frame( void )
{
	// Gamma composite: the scene lives in gl3_main_color; draw it to the
	// default framebuffer with the converted gamma shader (no-op maths when
	// r_gamma is 1, which it is by default).
	qglDisable( GL_SCISSOR_TEST );
	qglDisable( GL_DEPTH_TEST );
	qglDisable( GL_BLEND );
	qglDisable( GL_CULL_FACE );
	{
		// The window may be smaller than the render target when supersampling
		// or r_renderScale is in use; the composite covers the drawable.
		int dw = gl3_width, dh = gl3_height;
		GL3_PlatformDrawableSize( &dw, &dh );
		qglViewport( 0, 0, dw, dh );
	}

	if ( gl3_first_frame_checked < 60 ) {
		gl3_first_frame_checked++;
		GL3_CHECK( "present" );
	}

	if ( gl3_gamma_program ) {
		qglUseProgram( gl3_gamma_program );
		// Gamma constants are per-frame: r_gamma / r_greyscale / overbright can
		// change without restarting the renderer.
		if ( gl3_gamma_loc[0] >= 0 ) qglUniform1i( gl3_gamma_loc[0], 0 );
		if ( gl3_gamma_loc[1] >= 0 ) qglUniform1f( gl3_gamma_loc[1], 1.0f / r_gamma->value );
		if ( gl3_gamma_loc[2] >= 0 ) qglUniform1f( gl3_gamma_loc[2], (float)( 1 << tr.overbrightBits ) );
		if ( gl3_gamma_loc[3] >= 0 ) qglUniform1f( gl3_gamma_loc[3], r_greyscale->value );
		if ( gl3_gamma_loc[4] >= 0 ) qglUniform1i( gl3_gamma_loc[4], r_dither->integer );
		if ( gl3_gamma_loc[5] >= 0 ) qglUniform1i( gl3_gamma_loc[5], 255 );
		if ( gl3_gamma_loc[6] >= 0 ) qglUniform1i( gl3_gamma_loc[6], 255 );
		if ( gl3_gamma_loc[7] >= 0 ) qglUniform1i( gl3_gamma_loc[7], 255 );
		qglActiveTexture( GL_TEXTURE0 );
		qglBindTexture( GL_TEXTURE_2D, gl3_main_color );
		qglBindVertexArray( gl3_gamma_vao );
		qglDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
		qglBindVertexArray( gl3_vao );
		qglUseProgram( 0 );
	} else {
		// No gamma program: blit the colour target directly.
		qglBindFramebuffer( GL_READ_FRAMEBUFFER, gl3_main_fbo );
		qglBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
		qglBlitFramebuffer( 0, 0, gl3_width, gl3_height, 0, 0, gl3_width, gl3_height,
			GL_COLOR_BUFFER_BIT, GL_NEAREST );
		qglBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}

	GL3_PlatformSwapBuffers();
}

// ---------------------------------------------------------------------------
// Drawing
// ---------------------------------------------------------------------------

void vk_update_mvp( const float *m )
{
	if ( m )
		Com_Memcpy( gl3_mvp, m, sizeof( gl3_mvp ) );
	else
		GL3_DefaultMVP( gl3_mvp );
}

void vk_reset_descriptor( int index )
{
	if ( index >= 0 && index < VK_DESC_COUNT )
		gl3_cmd.descriptor_set.current[index] = (VkDescriptorSet)(uintptr_t)0;
}

void vk_update_descriptor( int index, VkDescriptorSet descriptor )
{
	if ( index >= 0 && index < VK_DESC_COUNT )
		gl3_cmd.descriptor_set.current[index] = descriptor;
}

void vk_update_descriptor_offset( int index, uint32_t offset )
{
	if ( index == VK_DESC_UNIFORM )
		gl3_cmd.descriptor_set.offset[index] = offset;
}

void vk_bind_pipeline( uint32_t pipeline )
{
	const Vk_Pipeline_Def *def;
	gl3_program_t *prog;

	if ( pipeline >= vk.pipelines_count ) {
		ri.Printf( PRINT_WARNING, "GLES3: bind_pipeline out of range %u\n", pipeline );
		return;
	}

	if ( (int)pipeline == gl3_pipeline && gl3_pipeline >= 0 ) {
		// Same def as the previous draw: state is already set.
	}

	gl3_pipeline = (int)pipeline;
	def = &vk.pipelines[pipeline].def;

	prog = GL3_GetProgram( def );
	if ( !prog )
		return;

	qglUseProgram( prog->program );
	GL3_SetState( def );

	// Per-pipeline constants expressible as uniforms.
	if ( prog->loc_identity_color >= 0 )
		qglUniform1f( prog->loc_identity_color, (float)def->color.rgb / 255.0f );
	if ( prog->loc_identity_alpha >= 0 )
		qglUniform1f( prog->loc_identity_alpha, (float)def->color.alpha / 255.0f );
	if ( prog->loc_abs_light >= 0 )
		qglUniform1i( prog->loc_abs_light, def->abs_light ? 1 : 0 );
	if ( prog->loc_acff >= 0 )
		qglUniform1i( prog->loc_acff, def->fog_stage ? def->acff : 0 );

	// Alpha test function from the state bits (same mapping as the Vulkan
	// specialization constants).
	if ( prog->loc_alpha_test_func >= 0 ) {
		unsigned int atest = def->state_bits & GLS_ATEST_BITS;
		int func = 0;
		float value = 0.0f;

		switch ( atest ) {
		case GLS_ATEST_GT_0:  func = 1; value = 0.0f; break;
		case GLS_ATEST_LT_80: func = 2; value = 0.5f; break;
		case GLS_ATEST_GE_80: func = 3; value = 0.5f; break;
		default: func = 0; value = 0.0f; break;
		}
		qglUniform1i( prog->loc_alpha_test_func, func );
		if ( prog->loc_alpha_test_value >= 0 )
			qglUniform1f( prog->loc_alpha_test_value, value );
	}

	// Vertex colours are always in the uniform block; the block layout does not
	// depend on the def, so only the offset matters for each draw.

	vk.cmd->last_pipeline = (VkPipeline)(uintptr_t)pipeline;
}

void vk_bind_geometry( uint32_t flags )
{
	gl3_geometry_flags = flags;
}

// Attribute layout mirrors the Vulkan input state: loc0 xyz, loc1 colour0,
// loc2 st0, loc3 st1, loc4 st2, loc5 normal, loc6 colour1, loc7 colour2.
void vk_bind_lighting( int stage, int bundle )
{
	(void)stage; (void)bundle;
}

// Tessellation flags -> vertex attribute set. The front end emits exactly the
// flags below (see tr_shade.c's tess_flags per stage type); a shader permutation
// only declares the locations its #ifdefs use, and a pointer for an unused
// location is harmless. This table is the contract the Vulkan backend's
// push_bind/push_attr switch encodes:
//
//   flags                          locations bound
//   TESS_XYZ                       0  (vec4 xyz)
//   TESS_RGBA0                     1  (RGBA8 vertex colour)
//   TESS_ST0                       2  (vec2)
//   TESS_ST1                       3  (vec2)
//   TESS_ST2                       4  (vec2)
//   TESS_NNN (env lighting)        5  (vec4 normal)
//   TESS_RGBA1                     6  (RGBA8)
//   TESS_RGBA2                     7  (RGBA8)
//
// 2D/UI stages use TESS_XYZ|TESS_ST0|TESS_RGBA0 and the 2D MVP computed in
// GL3_DefaultMVP. Colour-only stages use TESS_XYZ (loc 0) and the colour
// program. The lighting variant binds 0/1/2 in vk_bind_lighting's order.
static void GL3_BindTessellation( void )
{
	uint32_t flags = gl3_geometry_flags;
	int count = tess.numVertexes;
	uint32_t total = 0;
	uint32_t offset = 0;

	if ( count <= 0 )
		return;

	// Size the buffer: one slice per attribute the pipeline consumes.
	if ( flags & TESS_XYZ )   total += (uint32_t)count * sizeof( vec4_t );
	if ( flags & TESS_RGBA0 ) total += (uint32_t)count * sizeof( color4ub_t );
	if ( flags & TESS_ST0 )   total += (uint32_t)count * sizeof( vec2_t );
	if ( flags & TESS_ST1 )   total += (uint32_t)count * sizeof( vec2_t );
	if ( flags & TESS_ST2 )   total += (uint32_t)count * sizeof( vec2_t );
	if ( flags & TESS_NNN )   total += (uint32_t)count * sizeof( vec4_t );
	if ( flags & TESS_RGBA1 ) total += (uint32_t)count * sizeof( color4ub_t );
	if ( flags & TESS_RGBA2 ) total += (uint32_t)count * sizeof( color4ub_t );
	total = (uint32_t)PAD( total, 4 );

	qglBindVertexArray( gl3_vao );
	qglBindBuffer( GL_ARRAY_BUFFER, gl3_vbo );
	qglBufferData( GL_ARRAY_BUFFER, total ? (GLsizeiptr)total : 4, NULL, GL_STREAM_DRAW );

#define GL3_ATTR( loc, n, type, ptr, elemsize ) do { \
		GLsizeiptr bytes = (GLsizeiptr)count * (elemsize); \
		qglBufferSubData( GL_ARRAY_BUFFER, (GLintptr)offset, bytes, (const void *)( ptr ) ); \
		qglEnableVertexAttribArray( loc ); \
		qglVertexAttribPointer( loc, ( n ), ( type ), GL_FALSE, (GLsizei)( elemsize ), (const void *)(uintptr_t)offset ); \
		offset = (uint32_t)PAD( offset + (uint32_t)bytes, 4 ); \
	} while ( 0 )

	if ( flags & TESS_XYZ )   GL3_ATTR( 0, 4, GL_FLOAT,         &tess.xyz[0],                 sizeof( vec4_t ) );
	if ( flags & TESS_RGBA0 ) GL3_ATTR( 1, 4, GL_UNSIGNED_BYTE, tess.svars.colors[0][0].rgba,  sizeof( color4ub_t ) );
	if ( flags & TESS_ST0 )   GL3_ATTR( 2, 2, GL_FLOAT,         tess.svars.texcoordPtr[0],    sizeof( vec2_t ) );
	if ( flags & TESS_ST1 )   GL3_ATTR( 3, 2, GL_FLOAT,         tess.svars.texcoordPtr[1],    sizeof( vec2_t ) );
	if ( flags & TESS_ST2 )   GL3_ATTR( 4, 2, GL_FLOAT,         tess.svars.texcoordPtr[2],    sizeof( vec2_t ) );
	if ( flags & TESS_NNN )   GL3_ATTR( 5, 4, GL_FLOAT,         &tess.normal[0],              sizeof( vec4_t ) );
	if ( flags & TESS_RGBA1 ) GL3_ATTR( 6, 4, GL_UNSIGNED_BYTE, tess.svars.colors[1][0].rgba,  sizeof( color4ub_t ) );
	if ( flags & TESS_RGBA2 ) GL3_ATTR( 7, 4, GL_UNSIGNED_BYTE, tess.svars.colors[2][0].rgba,  sizeof( color4ub_t ) );

#undef GL3_ATTR
}

void vk_bind_index( void )
{
	vk_bind_index_ext( tess.numIndexes, tess.indexes );
}

void vk_bind_index_ext( const int numIndexes, const uint32_t *indexes )
{
	if ( !indexes || numIndexes <= 0 ) {
		gl3_cmd.num_indexes = 0;
		return;
	}

	qglBindBuffer( GL_ELEMENT_ARRAY_BUFFER, gl3_ibo );
	qglBufferData( GL_ELEMENT_ARRAY_BUFFER, numIndexes * (int)sizeof( glIndex_t ),
		indexes, GL_STREAM_DRAW );
	gl3_cmd.num_indexes = numIndexes;
}

uint32_t vk_tess_index( uint32_t numIndexes, const void *src )
{
	// Unused by this backend (indices go to gl3_ibo directly), kept for the
	// shared front end's VBO plumbing.
	(void)numIndexes; (void)src;
	return ~0U;
}

void vk_bind_index_buffer( VkBuffer buffer, uint32_t offset )
{
	(void)buffer; (void)offset;
}

void vk_draw_indexed( uint32_t indexCount, uint32_t firstIndex )
{
	if ( !indexCount )
		return;
	qglDrawElements( GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT,
		(const void *)(uintptr_t)( firstIndex * sizeof( glIndex_t ) ) );
}

static void GL3_BindTextureUnit( int unit, VkDescriptorSet descriptor )
{
	GLuint tex = GL3_TextureFromDescriptor( descriptor );

	if ( !tex )
		tex = tr.whiteImage ? (GLuint)(uintptr_t)tr.whiteImage->handle : 0;
	if ( !tex )
		return;

	qglActiveTexture( GL_TEXTURE0 + unit );
	qglBindTexture( GL_TEXTURE_2D, tex );
}

static void GL3_UploadUniforms( void )
{
	uint32_t offset = gl3_cmd.descriptor_set.offset[VK_DESC_UNIFORM];
	uint32_t size = sizeof( vkUniform_t );

	if ( offset + size > GL3_UNIFORM_STAGE_SIZE )
		offset = 0;

	qglBindBuffer( GL_UNIFORM_BUFFER, gl3_ubo );
	qglBufferSubData( GL_UNIFORM_BUFFER, 0, (GLsizeiptr)size, gl3_uniform_stage + offset );
	qglBindBufferBase( GL_UNIFORM_BUFFER, 0, gl3_ubo );
}

void vk_draw_geometry( Vk_Depth_Range depth_range, qboolean indexed )
{
	gl3_program_t *prog;
	const Vk_Pipeline_Def *def;

	if ( !gl3_initialized || gl3_pipeline < 0 || gl3_pipeline >= (int)vk.pipelines_count )
		return;

	def = &vk.pipelines[gl3_pipeline].def;

	if ( def->shader_type == TYPE_DOT )
		return;	// flares are not implemented for this backend

	prog = &gl3_programs[0];
	{
		int i;
		for ( i = 0; i < gl3_program_count; i++ ) {
			if ( gl3_programs[i].program && gl3_programs[i].shader_type == (int)def->shader_type &&
			     gl3_programs[i].fog_stage == (def->fog_stage ? 1 : 0) ) {
				prog = &gl3_programs[i];
				break;
			}
		}
	}

	GL3_UpdateViewport( depth_range );

	// Texture units: the front end bound image descriptors by role.
	GL3_BindTextureUnit( 0, gl3_cmd.descriptor_set.current[VK_DESC_TEXTURE0] );
	GL3_BindTextureUnit( 1, gl3_cmd.descriptor_set.current[VK_DESC_TEXTURE1] );
	GL3_BindTextureUnit( 2, gl3_cmd.descriptor_set.current[VK_DESC_TEXTURE2] );

	if ( def->fog_stage ) {
		// Deterministic fog source, not a heuristic: the front end records the
		// fog image in exactly one slot per path --
		//   TYPE_FOG_ONLY / lighting (FOG_DLIGHT)  -> VK_DESC_FOG_ONLY   (== 1)
		//   a fogged stage (fog collapse)          -> VK_DESC_FOG_COLLAPSE (== 4)
		// -- and the converted GLSL reads it at texture unit 3.
		if ( def->shader_type == TYPE_FOG_ONLY ||
		     def->shader_type == TYPE_SIGNLE_TEXTURE_LIGHTING ||
		     def->shader_type == TYPE_SIGNLE_TEXTURE_LIGHTING_LINEAR )
			GL3_BindTextureUnit( 3, gl3_cmd.descriptor_set.current[VK_DESC_FOG_ONLY] );
		else
			GL3_BindTextureUnit( 3, gl3_cmd.descriptor_set.current[VK_DESC_FOG_COLLAPSE] );
	}

	if ( prog->loc_mvp >= 0 )
		qglUniformMatrix4fv( prog->loc_mvp, 1, GL_FALSE, gl3_mvp );

	GL3_UploadUniforms();

	if ( prog->loc_texture0 >= 0 ) qglUniform1i( prog->loc_texture0, 0 );
	if ( prog->loc_texture1 >= 0 ) qglUniform1i( prog->loc_texture1, 1 );
	if ( prog->loc_texture2 >= 0 ) qglUniform1i( prog->loc_texture2, 2 );
	if ( prog->loc_fog_texture >= 0 ) qglUniform1i( prog->loc_fog_texture, 3 );

	if ( def->shader_type == TYPE_FOG_ONLY ) {
		// Fullscreen quad, no attributes: the converted fog.vert uses
		// gl_VertexID, so a plain 4-vertex strip is all it needs. GLES has no
		// default VAO, so a real empty VAO must be current.
		qglBindVertexArray( gl3_gamma_vao );
		qglDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
		qglBindVertexArray( gl3_vao );
		return;
	}

	if ( def->shader_type == TYPE_SIGNLE_TEXTURE_LIGHTING ||
	     def->shader_type == TYPE_SIGNLE_TEXTURE_LIGHTING_LINEAR ) {
		// The lighting pass binds its attributes through vk_bind_lighting; the
		// light program uses loc0 xyz, loc1 st, loc2 normal, which the caller
		// provides via the normal tessellation path for lit surfaces.
	}

	GL3_BindTessellation();

	if ( indexed ) {
		if ( gl3_cmd.num_indexes > 0 )
			qglDrawElements( GL_TRIANGLES, gl3_cmd.num_indexes, GL_UNSIGNED_INT, NULL );
	} else if ( tess.numVertexes > 0 ) {
		qglDrawArrays( GL_TRIANGLES, 0, tess.numVertexes );
	}
}

void vk_draw_dot( uint32_t storage_offset )
{
	(void)storage_offset;	// see TYPE_DOT note
}

void vk_read_pixels( byte *buffer, uint32_t width, uint32_t height )
{
	if ( !buffer )
		return;

	qglBindFramebuffer( GL_FRAMEBUFFER, gl3_main_fbo );
	qglReadPixels( 0, 0, (GLsizei)width, (GLsizei)height, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
	qglBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

qboolean vk_bloom( void )
{
	if ( r_bloom->integer && !gl3_bloom_warned ) {
		gl3_bloom_warned = qtrue;
		ri.Printf( PRINT_WARNING, "GLES3: bloom is not implemented by this backend; ignoring r_bloom\n" );
	}
	return qfalse;
}

qboolean vk_alloc_vbo( const byte *vbo_data, int vbo_size )
{
	(void)vbo_data; (void)vbo_size;
	return qfalse;	// static world VBO caching not implemented; dynamic path is used
}

const char *vk_format_string( VkFormat format )
{
	switch ( format ) {
	case VK_FORMAT_R8G8B8A8_UNORM: return "RGBA8";
	case VK_FORMAT_B8G8R8A8_UNORM: return "BGRA8";
	case VK_FORMAT_D24_UNORM_S8_UINT: return "D24S8";
	default: return "unknown";
	}
}

// ---------------------------------------------------------------------------
// VBO helpers: the shared front end calls these under USE_VBO. Static-geometry
// caching is not implemented, so they degrade to the dynamic path (the
// tessellation arrays are uploaded per draw above).
// ---------------------------------------------------------------------------

void VBO_PrepareQueues( void )
{
}

void VBO_RenderIBOItems( void )
{
}

void VBO_ClearQueue( void )
{
}

void VBO_UnBind( void )
{
#ifdef USE_VBO
	tess.vboIndex = 0;
#endif
}

void VBO_Flush( void )
{
}

void VBO_PushData( int itemIndex, shaderCommands_t *input )
{
	(void)itemIndex; (void)input;
}

void VBO_QueueItem( int itemIndex )
{
	(void)itemIndex;
}

// ---------------------------------------------------------------------------
// Module surface
// ---------------------------------------------------------------------------

/*
====================
renderer_backend_ready

Exported for the client's module probe (CL_PublishRendererModules). This
backend is still being brought up: the 2D/world draw path is implemented, but
nothing has been executed yet, so the menu must not offer it and GetRefAPI
(refuses below) keeps the client on Vulkan.
====================
*/
Q_EXPORT int renderer_backend_ready( void )
{
	// Opt-in test switch without a rebuild: `gles3_forceReady 1` (in the saved
	// config or on the command line with +set). The client calls this both from
	// its module probe (where `ri` is not filled yet -- then the default answer
	// stands) and from GetRefAPI (where it is), so the guard is required.
	if ( ri.Cvar_Get ) {
		cvar_t *force = ri.Cvar_Get( "gles3_forceReady", "0", CVAR_ARCHIVE );
		if ( force && force->integer )
			return 1;
	}
	return 0;
}

/*
====================
GL3_BackendStatus

One line for the module's startup message and for gfxinfo-style diagnostics.
====================
*/
const char *GL3_BackendStatus( void )
{
	if ( !gl3_backend_ok )
		return "GLES 3.2 backend: not initialized";
	return "GLES 3.2 backend: initialized, draw path implemented, unverified";
}

// ---------------------------------------------------------------------------
// Flares
//
// The Vulkan backend tests flare visibility by rendering a point sprite that
// writes a storage-buffer flag through the fragment shader (there is no depth
// readback in Vulkan) and reading the flag back the next frame. GLES 3.2 has no
// equivalent without a framebuffer-fetch extension, and the GLES backend does
// not render point sprites into the depth buffer, so flares are disabled here
// rather than faked. r_flares is off by default and stays off for this backend;
// AUDIT.md tracks a GLES visibility test as remaining work.
// ---------------------------------------------------------------------------

void R_ClearFlares( void )
{
}

void RB_AddFlare( void *surface, int fogNum, vec3_t point, vec3_t color, vec3_t normal )
{
	(void)surface; (void)fogNum; (void)point; (void)color; (void)normal;
}

void RB_RenderFlares( void )
{
}

// ---------------------------------------------------------------------------
// Static world-geometry VBO cache
//
// vk_vbo.c builds an interleaved world VBO for static surfaces. GLES uses the
// dynamic tessellation path instead (attributes are streamed per draw), so the
// world is rendered from the front end's arrays; this hook is a no-op.
// ---------------------------------------------------------------------------

void R_BuildWorldVBO( msurface_t *surf, int surfCount )
{
	(void)surf; (void)surfCount;
}
