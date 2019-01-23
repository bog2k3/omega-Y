#version 330 core

in vec3 pos;
in vec3 normal;
in vec3 color;
in vec2 uv[4];
in float texWeight[4];
 
out vec3 fWPos;
out vec3 fNormal;
out vec4 fColor;
out vec2 fUV[4];
out float fTexWeight[4];

uniform mat4 mPV;
 
void main() {
    gl_Position = mPV * vec4(pos, 1);
	fWPos = pos;
    fNormal = normal;
    fColor = vec4(color, 1);
    fUV[0] = uv[0];
	fUV[1] = uv[1];
	fUV[2] = uv[2];
	fUV[3] = uv[3];
	fTexWeight[0] = texWeight[0];
	fTexWeight[1] = texWeight[1];
	fTexWeight[2] = texWeight[2];
	fTexWeight[3] = texWeight[3];
}
