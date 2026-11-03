#version 320 es

// Converted from renderervk/shaders/gamma.vert for GLES 3.2 by
// renderergles3/shaders/convert_glsl.py. Do not edit by hand: edit the source
// template and re-run the script.
//
// GLES conversions applied: #version 320 es + precision qualifiers; Vulkan
// push-constant block -> `uniform mat4 mvp`; descriptor sets -> GL texture
// units (`layout(binding = N)`); specialization constants -> uniforms (the
// backend uploads all of them per draw, defaults listed inline below);
// gl_PerVertex and layout(depth_less) are GLSL ES built-ins.
//
precision highp float;
precision highp int;

layout(location = 0) out vec2 frag_tex_coord;

const vec2 v[4] = vec2[4](
	vec2(-1.0f, 1.0f),
	vec2(-1.0f,-1.0f),
	vec2( 1.0f, 1.0f),
	vec2( 1.0f,-1.0f)
);

const vec2 t[4] = vec2[4](
	vec2( 0.0f, 1.0f),
	vec2( 0.0f, 0.0f),
	vec2( 1.0f, 1.0f),
	vec2( 1.0f, 0.0f)
);



void main() {
	gl_Position = vec4( v[ gl_VertexID ], 0.0f, 1.0f );
	frag_tex_coord = t[ gl_VertexID ];
}
