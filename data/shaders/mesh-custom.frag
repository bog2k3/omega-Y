#version 330 core

#include common.glsl
#include underwater.glsl

in vec4 fColor;
in vec2 fUV;
in vec3 fNormal;
in vec3 fWPos;

uniform sampler2D tex1;

void main() {
	vec4 dummy = fColor + vec4(fUV, 0, 0) + vec4(fNormal, 0);

	float eyeDist = length(eyePos - fWPos);
	bool underwater = subspace < 0;

	// compute lighting
	vec3 light = underwater ? computeLightingUnderwater(fWPos, normalize(fNormal), eyeDist) : computeLightingAboveWater(normalize(fNormal));

	vec3 color = light * fColor.xyz + dummy.xyz * 0.001;

	// water fog:
	if (underwater)
		color = computeWaterFog(fWPos, color, eyeDist);

	vec4 final = vec4(color, 1);
	if (bRefraction > 0 || bReflection > 0)
		final.a = computeZValue(gl_FragCoord);

	gl_FragColor = final;
}
