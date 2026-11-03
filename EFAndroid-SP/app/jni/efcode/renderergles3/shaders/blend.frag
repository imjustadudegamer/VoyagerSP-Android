#version 320 es

// Converted from renderervk/shaders/blend.frag for GLES 3.2 by
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

layout(binding = 0) uniform sampler2D texture0;
layout(binding = 1) uniform sampler2D texture1;
layout(binding = 2) uniform sampler2D texture2;
layout(binding = 3) uniform sampler2D texture3;

layout(location = 0) in vec2 tex_coord;

layout(location = 0) out vec4 out_color;

//uniform float gamma; // was constant_id=0 default 1.0
//uniform float obScale; // was constant_id=1 default 2.0
//uniform float greyscale; // was constant_id=2 default 0.0
//uniform float threshold; // was constant_id=3 default 0.6
uniform float factor; // was constant_id=4 default 0.5

//const vec3 sRGB = vec3( 0.2126, 0.7152, 0.0722 );

void main()
{
	vec3 base = texture(texture0, tex_coord).rgb + texture(texture1, tex_coord).rgb + texture(texture2, tex_coord).rgb + texture(texture3, tex_coord).rgb;

	if ( dot(base,base) == 0.0 )
	{
		discard;
	}

	out_color = vec4( base * factor, 0.0 );
}
