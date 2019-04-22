#version 330 core

#include common.glsl

in vec3 pos;
in vec3 normal;
in vec3 color;
in vec2 uv[5];
in vec4 texBlendFactor;

uniform mat4 matW;

out FragData {
	vec3 wPos;
	vec3 normal;
	vec3 color;
	vec2 uv[5];
	vec4 texBlendFactor;
} vertexOut;

void main() {
	vec3 wPos = pos; //(matW * vec4(pos, 1)).xyz;
	gl_Position = matPV * vec4(wPos, 1);
	gl_ClipDistance[0] = 1;

	vertexOut.wPos = wPos;
	vertexOut.normal = normal;
	vertexOut.color = color;
	vertexOut.uv = uv;
	vertexOut.texBlendFactor = texBlendFactor;
}
