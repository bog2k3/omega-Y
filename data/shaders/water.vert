#version 330 core

in vec3 pos;
in vec2 uv;
in float fog;

out vec3 fWPos;
out vec2 fUV;
out float fFog;

uniform mat4 mPV;		// proj*view

void main() {
   	gl_Position = mPV * vec4(pos, 1);
	fWPos = pos;
   	fFog = fog;
   	fUV = uv;
}
