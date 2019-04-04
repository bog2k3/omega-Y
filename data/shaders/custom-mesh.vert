#version 330 core

#include common.glsl
#include underwater.glsl

in vec3 vPos;
in vec3 vNormal;
in vec4 vColor;
in vec2 vUV1;

out vec4 fColor;
out vec2 fUV1;
out vec3 fNormal;

uniform mat4 mPVW;

void main() {
	vec3 wPos = vPos;
	if (bRefraction > 0) {
		// refract the position of the vertex
		wPos = refractPos(wPos, eyePos);
	}
    gl_Position = mPVW * vec4(wPos, 1);
	gl_ClipDistance[0] = vPos.y * sign(subspace) + bRefraction * 0.2;

    fNormal = (mPVW * vec4(vNormal, 0)).xyz;
    fColor = vColor;
    fUV1 = vUV1;
}
