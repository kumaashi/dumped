#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location=0) out vec4 FragColor0;


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

void main() {
	vec2 uv = gl_FragCoord.xy / ubuf.resolution.xy;
	vec4 color_tex0_ret = texture(color_tex0, uv);
	vec4 color_tex1_ret = texture(color_tex1, uv);
	vec4 color_tex2_ret = texture(color_tex2, uv);
	vec4 color_tex3_ret = texture(color_tex3, uv);
	FragColor0 = (color_tex0_ret + color_tex1_ret + color_tex2_ret + color_tex3_ret) / 4.0;
}

