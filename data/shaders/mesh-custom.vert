#version 330 core

#include common.glsl
#include underwater.glsl

in vec3 pos;
in vec3 normal;
in vec4 color;
in vec2 uv;

out vec4 fColor;
out vec2 fUV;
out vec3 fNormal;
out vec3 fWPos;

uniform mat4 mPV;
uniform mat4 mW;

void main() {
	vec3 wPos = (mW * vec4(pos, 1)).xyz;
	if (bRefraction > 0) {
		// refract the position of the vertex
		wPos = refractPos(wPos, eyePos);
	}
	fWPos = wPos;
	gl_Position = mPV * vec4(wPos, 1);
	gl_ClipDistance[0] = wPos.y * sign(subspace);// + bRefraction * 0.2;

    fNormal = (mW * vec4(normal, 0)).xyz;
    fColor = color;
    fUV = uv;
}
