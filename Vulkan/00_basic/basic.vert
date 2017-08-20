#version 450 core
layout(location=0) in  vec3 pos;
layout(location=1) in  vec3 nor;
layout(location=2) in  vec2 uv;
layout(location=3) in  vec4 color;

layout(std140, set = 0, binding = 0) uniform buf {
	vec4 resolution;
	vec4 basecolor;
} ubuf;

void main() {
float x = sin(ubuf.basecolor.x);
	gl_Position = vec4(pos * vec3(1 + x, -1, 0), 1.0);
}
