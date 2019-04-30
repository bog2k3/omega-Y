#version 330 core

#include common.glsl
 
in vec3 pos;
 
out vec3 fUV;
 
void main(){
	vec4 projected = matPV * vec4(pos, 0);
	projected.z = 1.0;
	projected.w = 1;
    gl_Position = projected;
    fUV = pos;
}
