#version 330 core

in vec2 fUV;

uniform sampler2D tex0;

void main() {
	gl_FragColor = texture(tex0, fUV);
}
