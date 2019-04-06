#version 330 core

#include common.glsl
#include underwater.glsl

in vec3 pos;
in vec3 normal;
in vec3 color;
in vec2 uv[5];
in vec4 texBlendFactor;

out VertexData {
	vec3 fWPos;
	vec3 fNormal;
	vec4 fColor;
	vec2 fUV[5];
	vec4 fTexBlendFactor;
} vertexOut;

uniform mat4 mPV;

void main() {
	vec3 wPos = pos;
	if (bRefraction > 0) {
		// refract the position of the vertex
		wPos = refractPos(wPos, eyePos);
	}
	gl_Position = mPV * vec4(wPos, 1);
	gl_ClipDistance[0] = pos.y * sign(subspace);
	vertexOut.fWPos.xyz = pos; // ?wPos?
	//fWPos.w = gl_Position.z;
	vertexOut.fNormal = normal;
	vertexOut.fColor = vec4(color, 1);
	vertexOut.fUV[0] = uv[0];
	vertexOut.fUV[1] = uv[1];
	vertexOut.fUV[2] = uv[2];
	vertexOut.fUV[3] = uv[3];
	vertexOut.fUV[4] = uv[4];
	vertexOut.fTexBlendFactor = texBlendFactor;
}
