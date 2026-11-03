#version 320 es

// Converted from renderervk/shaders/color.frag for GLES 3.2 by
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

layout(location = 0) out vec4 out_color;

uniform int color_mode; // was constant_id=4 default 0

void main()
{
	if ( color_mode == 1 )
		out_color = vec4( 1.0, 1.0, 1.0, 1.0 ); // white
	else
	if ( color_mode == 2 )
		out_color = vec4( 0.2, 1.0, 0.2, 1.0 ); // green
	else
	if ( color_mode == 3 )
		out_color = vec4( 1.0, 0.33, 0.2, 1.0 ); // red
	else
		out_color = vec4( 0.0, 0.0, 0.0, 1.0 ); // black
}
