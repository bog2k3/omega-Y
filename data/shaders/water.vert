#version 330 core

in vec3 pos;
in vec2 uv;
in float fog;

out vec3 fWPos;
out vec2 fUV;
out float fFog;
out vec3 fScreenUV;

uniform mat4 mPV;		// proj*view

void main() {
	vec3 offs = vec3(0.0);
	//offs = vec3(0.0, -0.1, 0.0);
   	gl_Position = mPV * vec4(pos + offs, 1);
	fScreenUV = gl_Position.xyw;
	fWPos = pos;
   	fFog = fog;
   	fUV = uv;
}
