#version 330 core

in vec3 pos;
in vec3 color;
in vec2 uv;
 
out vec3 fWPos;
out vec3 fColor;
out vec2 fUV;

uniform mat4 mPV;
 
void main() {
   	gl_Position = mPV * vec4(pos, 1);
	fWPos = pos;
   	fColor = color;
   	fUV = uv;
}
