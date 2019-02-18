#version 330 core
 
in vec3 pos;

uniform mat4 mVP;
 
out vec3 fUV;
 
void main(){
    gl_Position = mVP * vec4(pos, 1);
    fUV = pos;
}
