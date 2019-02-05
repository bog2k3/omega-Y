#version 330 core
 
in vec3 pos;
in vec2 uv;

uniform mat4 mVP;
 
out vec2 fUV;
 
void main(){
    gl_Position = mVP * vec4(pos, 1);
    fUV = uv;
}
