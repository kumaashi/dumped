#version 450 core
layout(location=0) in  vec3 pos;
layout(location=1) in  vec3 nor;
layout(location=2) in  vec2 uv;
layout(location=3) in  vec4 color;

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
	gl_Position = ubuf.proj * ubuf.view * vec4(pos, 1.0);
}
