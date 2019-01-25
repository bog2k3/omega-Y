#version 330 core

in vec3 fWPos;
in vec3 fNormal;
in vec4 fColor;
in vec2 fUV[4];
in vec3 fTexBlendFactor;

uniform sampler2D tex[4];

void main() {
	float lowFreqFactor = 0.05;
	// sample textures at two frequencies:
	vec4 t0 = texture(tex[0], fUV[0]);
	vec4 t0low = texture(tex[0], fUV[0] * lowFreqFactor);
	vec4 t1 = texture(tex[1], fUV[1]);
	vec4 t1low = texture(tex[1], fUV[1] * lowFreqFactor);
	vec4 t2 = texture(tex[2], fUV[2]);
	vec4 t2low = texture(tex[2], fUV[2] * lowFreqFactor);
	vec4 t3 = texture(tex[3], fUV[3]);
	vec4 t3low = texture(tex[3], fUV[3] * lowFreqFactor);
	
	// mix texture frequencies
	t0 = (t0 * t0low) * 2;
	t1 = (t1 * t1low) * 2;
	t2 = (t2 * t2low) * 2;
	t3 = (t3 * t3low) * 2;

	// blend the textures:
	vec4 t01 = mix(t0, t1, fTexBlendFactor.x);
	vec4 t23 = mix(t2, t3, fTexBlendFactor.y);
	vec4 tFinal = vec4(mix(t01, t23, 1.0 - fTexBlendFactor.z).xyz, 1.0);

	// compute lighting
	//vec3 lightPoint = vec3(0.0, 30.0, 0.0);
	vec3 lightDir = normalize(vec3(1.0, -1.5, -0.3));
	//vec3 lightVec = fWPos - lightPoint;
	//float lightDist = length(lightVec);
	//vec3 lightDir = lightVec / lightDist;
	float light = dot(-lightDir, normalize(fNormal));
	//float falloff = 1000.0 / (lightDist*lightDist);
	//float falloff = 25.0 / lightDist;
	float falloff = 1.0;

	//tFinal = vec4(1.0, 1.0, 1.0, 1.0);

	vec4 final = vec4(light * falloff * (fColor * tFinal).xyz, 1.0);
	
	//final = vec4(t2.xyz, 1.0) + 0.01 * final;

	gl_FragColor = final;
}
