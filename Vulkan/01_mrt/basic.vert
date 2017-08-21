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
	vec4 instance[1024];
} ubuf;

layout(location=0) out vec4 v_nor;
layout(location=1) out vec4 v_pos;
layout(location=2) out vec4 v_col;

void main() {

	vec3  L = normalize(vec3(-1,-1,-1));
	vec4 ipos      = ubuf.instance[gl_InstanceIndex];
	vec3 trans_pos = pos + ipos.xyz * 13.0;
	vec4 wvpPos    = ubuf.proj * ubuf.view * vec4(trans_pos, 1.0);
	vec3  N        = nor;
	vec3  V        = normalize(-pos);
	vec3  H        = normalize(L + V);
	float D        = max(0.0, dot(N, L));
	float S        = pow(max(0.0, dot(H, L)), 8.0);
	vec3  C        = normalize(vec3(1,2,3));
	v_nor          = vec4(nor, 1.0);
	v_pos          = wvpPos;
	v_col          = vec4(S * C + D * C, 1.0);
	gl_Position    = wvpPos;
}
