#version 320 es

// Converted from renderervk/shaders/bloom.frag for GLES 3.2 by
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

layout(location = 0) in vec2 frag_tex_coord;

layout(location = 0) out vec4 out_color;

//uniform float gamma; // was constant_id=0 default 1.0
//uniform float obScale; // was constant_id=1 default 2.0
//uniform float greyscale; // was constant_id=2 default 0.0
uniform float threshold; // was constant_id=3 default 0.6
//uniform float factor; // was constant_id=4 default 0.5
uniform int extract_mode; // was constant_id=5 default 0
uniform int base_modulate; // was constant_id=6 default 0

//const vec3 sRGB = vec3( 0.2126, 0.7152, 0.0722 );

void main() {
	vec3 base = texture(texture0, frag_tex_coord).rgb;

	if ( extract_mode == 1 ) // (r+g+b)/3 >= threshold
	{
		if ( (base.r + base.g + base.b) * 0.33333333 >= threshold)
		{
			if ( base_modulate != 0 )
			{
				if ( base_modulate == 1 )
				{
					base *= base;
				}
				else
				{
					const vec3 luma = vec3( 0.2126, 0.7152, 0.0722 );
					base *= dot( luma, base.rgb );
				}
			}
			out_color = vec4( base, 1.0 );
		}
		else
		{
			out_color = vec4( 0.0 );
		}
	}
	else if ( extract_mode == 2 ) // luma(r,g,b) >= threshold
	{
		const vec3 luma = vec3( 0.2126, 0.7152, 0.0722 );
		float v = dot( luma, base.rgb );
		if ( v >= threshold )
		{
			if ( base_modulate != 0 )
			{
				if ( base_modulate == 1 )
				{
					base *= base;
				}
				else
				{
					base *= v;
				}
			}
			out_color = vec4( base, 1.0 );
		}
		else
		{
			out_color = vec4( 0.0 );
		}
	}
	else // default case
	{
		if ( base.r >= threshold || base.g >= threshold || base.b >= threshold ) // (r|g|b) >= threshold
		{
			if ( base_modulate != 0 )
			{
				if ( base_modulate == 1 )
				{
					base *= base;
				}
				else
				{
					const vec3 luma = vec3( 0.2126, 0.7152, 0.0722 );
					base *= dot( luma, base.rgb );
				}
			}
			out_color = vec4( base, 1.0 );
		}
		else
		{
			out_color = vec4( 0.0 );
		}
	}
}
