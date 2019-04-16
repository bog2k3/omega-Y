#version 330 core

uniform float blendFactor;
uniform int blendMode;
uniform int mulTextureAlpha;
uniform vec4 blendColor;
uniform sampler2D texPicture;

in VertexData {
	vec2 uv;
} fragIn;

const int BLEND_MODE_NORMAL = 0;
const int BLEND_MODE_ADD = 1;
const int BLEND_MODE_MULTIPLY = 2;

void main() {
	vec4 texColor = texture(texPicture, fragIn.uv);
	float factor = blendFactor;
	factor *= mulTextureAlpha > 0 ? texColor.a : 1.0;
	
	vec4 color = blendColor;
	
	if (blendMode == BLEND_MODE_NORMAL)
		color = mix(color, texColor, factor);
	else if (blendMode == BLEND_MODE_ADD)
		color += texColor * factor;
	else if (blendMode == BLEND_MODE_MULTIPLY)
		color *= texColor * factor;
	
	gl_FragColor = color;
}