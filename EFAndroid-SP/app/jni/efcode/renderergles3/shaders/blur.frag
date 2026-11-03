#version 320 es

// Converted from renderervk/shaders/blur.frag for GLES 3.2 by
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

// 3-tap gaussian blur 
// exploiting linear filtering with -1.2 0 +1.2 texture offsets and 5 6 5 weighting
// to emulate 5-tap blur

layout(binding = 0) uniform sampler2D texture0;

layout(location = 0) in vec2 tex_coord0;

layout(location = 0) out vec4 out_color;

uniform float texoffset_x; // was constant_id=0 default 0.0
uniform float texoffset_y; // was constant_id=1 default 0.0

void main()
{
	vec2 tex_coord1 = tex_coord0;
	vec2 tex_coord2 = tex_coord0;

	tex_coord1.x += texoffset_x;
	tex_coord1.y += texoffset_y;

	tex_coord2.x -= texoffset_x;
	tex_coord2.y -= texoffset_y;

	vec3 base = texture(texture0, tex_coord0).rgb * (6.0 / 16.0)
		+ texture(texture0, tex_coord1).rgb * (5.0 / 16.0)
		+ texture(texture0, tex_coord2).rgb * (5.0 / 16.0);

	out_color = vec4( base, 1.0 );
}
