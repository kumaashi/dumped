#version 450


layout(location=0) out vec4 FragColor0;
layout(location=1) out vec4 FragColor1;
layout(location=2) out vec4 FragColor2;
layout(location=3) out vec4 FragColor3;


layout(location=0) in vec4 v_nor;
layout(location=1) in vec4 v_pos;
layout(location=2) in vec4 v_col;

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


void main() {
	FragColor0 = vec4(v_col.xyz, 1.0);
	FragColor1 = vec4(v_nor.xyz, 1.0);
	FragColor2 = v_pos;
	FragColor3 = vec4(gl_FragCoord.z, 0.2, 0.3, 1.0);
}
