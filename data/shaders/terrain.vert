#version 330 core

#include common.glsl
#include underwater.glsl

in vec3 pos;
in vec3 normal;
in vec3 color;
in vec2 uv[5];
in vec4 texBlendFactor;

out vec3 fWPos;
out vec3 fNormal;
out vec4 fColor;
out vec2 fUV[5];
out vec4 fTexBlendFactor;

uniform mat4 mPV;
uniform float subspace;	// represents the subspace we're rendering: +1 above water, -1 below water
uniform int bRefraction;
uniform vec3 eyePos;

void main() {
	vec3 wPos = pos;
	if (bRefraction > 0) {
		// refract the position of the vertex
		wPos = refractPos(wPos, eyePos);
	}
	gl_Position = mPV * vec4(wPos, 1);
	gl_ClipDistance[0] = pos.y * sign(subspace) + bRefraction * 0.2;
	fWPos.xyz = pos; // ?wPos?
	//fWPos.w = gl_Position.z;
	fNormal = normal;
	fColor = vec4(color, 1);
	fUV[0] = uv[0];
	fUV[1] = uv[1];
	fUV[2] = uv[2];
	fUV[3] = uv[3];
	fUV[4] = uv[4];
	fTexBlendFactor = texBlendFactor;
}
