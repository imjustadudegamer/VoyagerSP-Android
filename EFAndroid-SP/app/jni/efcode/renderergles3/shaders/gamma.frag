#version 320 es

// Converted from renderervk/shaders/gamma.frag for GLES 3.2 by
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

uniform float gamma; // was constant_id=0 default 1.0
uniform float obScale; // was constant_id=1 default 2.0
uniform float greyscale; // was constant_id=2 default 0.0
//
uniform int ditherMode; // was constant_id=7 default 0 // 0 - disabled, 1 - ordered
uniform int depth_r; // was constant_id=8 default 255
uniform int depth_g; // was constant_id=9 default 255
uniform int depth_b; // was constant_id=10 default 255

const vec3 sRGB = vec3( 0.2126, 0.7152, 0.0722 );

const int bayerSize = 8;
const float bayerMatrix[bayerSize * bayerSize] = float[bayerSize * bayerSize](
	0.0,  32.0, 8.0,  40.0, 2.0,  34.0, 10.0, 42.0,
	48.0, 16.0, 56.0, 24.0, 50.0, 18.0, 58.0, 26.0,
	12.0, 44.0, 4.0,  36.0, 14.0, 46.0, 6.0,  38.0,
	60.0, 28.0, 52.0, 20.0, 62.0, 30.0, 54.0, 22.0,
	3.0,  35.0, 11.0, 43.0, 1.0,  33.0, 9.0,  41.0,
	51.0, 19.0, 59.0, 27.0, 49.0, 17.0, 57.0, 25.0,
	15.0, 47.0, 7.0,  39.0, 13.0, 45.0, 5.0,  37.0,
	63.0, 31.0, 55.0, 23.0, 61.0, 29.0, 53.0, 21.0
);

float threshold() {
	ivec2 coordDenormalized = ivec2(gl_FragCoord.xy);
	ivec2 bayerCoord = coordDenormalized % bayerSize;
	float bayerSample = bayerMatrix[bayerCoord.x + bayerCoord.y * bayerSize];
	float threshold = (bayerSample + 0.5) / float(bayerSize * bayerSize);
	return threshold;
}

vec3 dither(vec3 color) {
	vec3 depth = vec3(depth_r, depth_g, depth_b);
	vec3 cDenormalized = color * depth;
	vec3 cLow = floor(cDenormalized);
	vec3 cFractional = cDenormalized - cLow;
	vec3 cDithered = cLow + step(threshold(), cFractional);
	return cDithered / depth;
}

void main() {
	vec3 base = texture(texture0, frag_tex_coord).rgb;

	if ( greyscale == 1.0 )
	{
		base = vec3(dot(base, sRGB));
	}
	else if ( greyscale != 0.0 )
	{
		vec3 luma = vec3(dot(base, sRGB));
		base = mix(base, luma, greyscale);
	}

	if ( gamma != 1.0 )
	{
		out_color = vec4(pow(base, vec3(gamma)) * obScale, 1);
	}
	else
	{
		out_color = vec4(base * obScale, 1);
	}

	if ( ditherMode == 1 ) {
		out_color.rgb = dither(out_color.rgb);
	}
}
