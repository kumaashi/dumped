#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable


layout(std140, set = 0, binding = 0) uniform buf {
	vec4 time;
	vec4 resolution;
	vec4 basecolor;
	vec4 reserve;
	mat4 world;
	mat4 view;
	mat4 proj;
} ubuf;


layout(binding = 1) uniform sampler2D color_tex0;
layout(binding = 2) uniform sampler2D color_tex1;
layout(binding = 3) uniform sampler2D color_tex2;
layout(binding = 4) uniform sampler2D color_tex3;


layout(location=0) out vec4 FragColor0;


vec4 GetColor(vec2 uv) {
	vec4 color    = texture(color_tex0, uv);
	vec4 normal   = texture(color_tex1, uv);
	vec4 position = texture(color_tex2, uv);
	vec4 other    = texture(color_tex3, uv);
	vec4 ret = color;
	if(uv.x > 0.5) {
		ret    = normal;
		if(uv.y > 0.5) {
			ret    = vec4(normalize(position.yzx), 1.0);
		}
	} else {
		if(uv.y < 0.5) {
			ret = vec4(pow(other.x, 32.0));
		}
	}
	return ret;
}

#define BLUR_POWER 12.0



void main() {
	vec2 uv = gl_FragCoord.xy / ubuf.resolution.xy;
	vec2 duv = uv * 2.0 - 1.0;
	vec2 center = ubuf.resolution.xy * 0.5;
	float vig = distance(center, gl_FragCoord.xy) / ubuf.resolution.x;
	vec2 pixel_size = 1.0 / (ubuf.resolution.xy + 0.5) * (vig * BLUR_POWER);
	
	vec4 col1 = GetColor( uv );
	vec4 col2 = GetColor( uv + vec2(0.0, pixel_size.y));
	vec4 col3 = GetColor( uv + vec2(pixel_size.x, 0.0));
	vec4 col4 = GetColor( uv + pixel_size);
	vec2 slide = vec2(0.005, 0.0) * vig + vec2(0.001, 0.0);
	vec4 col5 = clamp(GetColor(uv - slide), vec4(0.0), vec4(1.0));
	vec4 col6 = clamp(GetColor(uv + slide), vec4(0.0), vec4(1.0));
	col1.r += col5.r;
	col1.b += col6.b;

	col2.r += col5.r;
	col2.b += col6.b;

	col3.r += col5.r;
	col3.b += col6.b;

	col4.r += col5.r;
	col4.b += col6.b;
	col1 = clamp(col1, vec4(0.0), vec4(1.0));
	col2 = clamp(col2, vec4(0.0), vec4(1.0));
	col3 = clamp(col3, vec4(0.0), vec4(1.0));
	col4 = clamp(col4, vec4(0.0), vec4(1.0));

	FragColor0 = (col1 + col2 + col3 + col4) * 0.25;
	FragColor0 *= (1.0 - dot(duv * 0.5, duv));
}

