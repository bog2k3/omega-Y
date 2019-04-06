#version 330 core

in vec3 pos;
in float fog;

out vec3 fWPos;
out float fFog;
out vec3 fScreenUV;

uniform mat4 mPV;		// proj*view

void main() {
   	gl_Position = mPV * vec4(pos, 1);
	fScreenUV = gl_Position.xyw;
	fWPos = pos;
   	fFog = fog;
}
