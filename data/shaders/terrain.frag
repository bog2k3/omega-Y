#version 330 core

#include water-surface.glsl
#include underwater.glsl

in VertexData {
	vec3 fWPos;
	vec3 fNormal;
	vec4 fColor;
	vec2 fUV[5];
	vec4 fTexBlendFactor;
} vtx;

uniform sampler2D tex[5];

void main() {
	float lowFreqFactor = 0.05;
	// sample textures at two frequencies:
	vec4 t0 = texture(tex[0], vtx.fUV[0]);
	vec4 t0low = texture(tex[0], vtx.fUV[0] * lowFreqFactor);
	vec4 t1 = texture(tex[1], vtx.fUV[1]);
	vec4 t1low = texture(tex[1], vtx.fUV[1] * lowFreqFactor);
	vec4 t2 = texture(tex[2], vtx.fUV[2]);
	vec4 t2low = texture(tex[2], vtx.fUV[2] * lowFreqFactor);
	vec4 t3 = texture(tex[3], vtx.fUV[3]);
	vec4 t3low = texture(tex[3], vtx.fUV[3] * lowFreqFactor);
	vec4 t4 = texture(tex[4], vtx.fUV[4]);
	vec4 t4low = texture(tex[4], vtx.fUV[4] * lowFreqFactor);

	// mix texture frequencies
	t0 = t0 * t0low * 2.5;
	t1 = t1 * t1low * 2.5;
	t2 = t2 * t2low * 2.5;
	t3 = t3 * t3low * 2.5;
	t4 = t4 * t4low * 2.5;

	// blend the textures:
	vec4 t01 = mix(t0, t1, clamp(vtx.fTexBlendFactor.x, 0.0, 1.0)); // dirt and grass
	vec4 t23 = mix(t2, t3, clamp(vtx.fTexBlendFactor.y, 0.0, 1.0)); // rock 1 and rock 2
	vec4 tGrassOrSand = vec4(mix(t01, t4, clamp(vtx.fTexBlendFactor.w, 0.0, 1.0)).xyz, 1.0); // grass/dirt and sand
	vec4 tFinal = vec4(mix(tGrassOrSand, t23, 1.0 - clamp(vtx.fTexBlendFactor.z, 0.0, 1.0)).xyz, 1.0);
	//tFinal = vec4(clamp(fTexBlendFactor.x, 0.0, 1.0)) + tFinal*0.001;

	float eyeDist = length(eyePos - vtx.fWPos);

	bool underwater = subspace < 0;

	// compute lighting
	vec3 light = underwater ? computeLightingUnderwater(vtx.fWPos, normalize(vtx.fNormal), eyeDist) : computeLightingAboveWater(normalize(vtx.fNormal));

	vec3 color = light * (vtx.fColor * tFinal).xyz;

	// water fog:
	if (underwater)
		color = computeWaterFog(vtx.fWPos, color, eyeDist);

	vec4 final = vec4(color, 1);
	if (bRefraction > 0 || bReflection > 0)
		final.a = computeZValue(gl_FragCoord);

	gl_FragColor = final;
}
