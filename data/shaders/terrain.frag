in vec3 fNormal;
in vec4 fColor;
in vec2 fUV[4];
in float fTexWeight[4];

uniform sampler2D tex[4];

void main() {
	vec4 dummy = vec4(fUV[0].x, fUV[1].x, fUV[2].x, fUV[3].x) + vec4(fNormal, 0) + vec4(fTexWeight[0], fTexWeight[1], fTexWeight[2], fTexWeight[3]);
	gl_FragColor = fColor + dummy*0.01;// * texture(tex1, fUV1);
}
