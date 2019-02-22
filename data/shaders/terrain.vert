#version 330 core

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

void main() {
	gl_Position = mPV * vec4(pos, 1);
	gl_ClipDistance[0] = pos.y * sign(subspace);
	fWPos = pos;
	fNormal = normal;
	fColor = vec4(color, 1);
	fUV[0] = uv[0];
	fUV[1] = uv[1];
	fUV[2] = uv[2];
	fUV[3] = uv[3];
	fUV[4] = uv[4];
	fTexBlendFactor = texBlendFactor;
}
