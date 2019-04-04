#version 330 core

#include common.glsl
#include water-surface.glsl

in vec4 fColor;
in vec2 fUV1;
in vec3 fNormal;

uniform sampler2D tex1;

void main() {
	vec4 dummy = fColor + vec4(fUV1, 0, 0) + vec4(fNormal, 0);
	vec4 final = fColor + dummy*0.01;// * texture(tex1, fUV1);

	final.a = clamp((gl_FragCoord.z / gl_FragCoord.w - Zn) / (Zf - Zn), 0.0, 1.0); // fragment's z

	gl_FragColor = final;
}
