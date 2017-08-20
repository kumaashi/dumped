#version 450


layout(location=0) out vec4 FragColor0;
layout(location=1) out vec4 FragColor1;
layout(location=2) out vec4 FragColor2;
layout(location=3) out vec4 FragColor3;

layout(binding = 1) uniform sampler2D color_tex0;
layout(binding = 2) uniform sampler2D color_tex1;
layout(binding = 3) uniform sampler2D color_tex2;
layout(binding = 4) uniform sampler2D color_tex3;


layout(std140, set = 0, binding = 0) uniform buf {
	vec4 time;
	vec4 resolution;
	vec4 basecolor;
	vec4 reserve;
	mat4 world;
	mat4 view;
	mat4 proj;
} ubuf;


void main() {
	vec4 col = ubuf.basecolor;
	col.x = abs(sin(col.x));
	FragColor0 = vec4(1, 1, 0, 1);
	FragColor1 = vec4(1, 0, 1, 1);
	FragColor2 = vec4(1, 0, 0, 1);
	FragColor3 = vec4(0, 0, 1, 1);
}
