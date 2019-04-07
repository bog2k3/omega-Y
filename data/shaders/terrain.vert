#version 330 core

#include common.glsl

in vec3 pos;
in vec3 normal;
in vec3 color;
in vec2 uv[5];
in vec4 texBlendFactor;

out VertexData {
	vec3 normal;
	vec3 color;
	vec2 uv[5];
	vec4 texBlendFactor;
} vertexOut;

void main() {
	gl_Position = vec4(pos, 1);
	gl_ClipDistance[0] = pos.y * sign(subspace) + bReflection * 0.1 + bRefraction * 0.01;

	vertexOut.normal = normal;
	vertexOut.color = color;
	vertexOut.uv = uv;
	vertexOut.texBlendFactor = texBlendFactor;
}
