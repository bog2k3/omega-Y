#version 330 core

in vec3 fWPos;
in vec3 fNormal;
in vec4 fColor;
in vec2 fUV[5];
in vec4 fTexBlendFactor;

uniform sampler2D tex[5];

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
	vec4 t4 = texture(tex[4], fUV[4]);
	vec4 t4low = texture(tex[4], fUV[4] * lowFreqFactor);
	
	// mix texture frequencies
	t0 = t0 * t0low * 2.5;
	t1 = t1 * t1low * 2.5;
	t2 = t2 * t2low * 2.5;
	t3 = t3 * t3low * 2.5;
	t4 = t4 * t4low * 2.5;

	// blend the textures:
	vec4 t01 = mix(t0, t1, clamp(fTexBlendFactor.x, 0.0, 1.0));
	vec4 t23 = mix(t2, t3, clamp(fTexBlendFactor.y, 0.0, 1.0));
	vec4 tGround = vec4(mix(t01, t23, 1.0 - clamp(fTexBlendFactor.z, 0.0, 1.0)).xyz, 1.0);
	vec4 tFinal = vec4(mix(tGround, t4, clamp(fTexBlendFactor.w, 0.0, 1.0)).xyz, 1.0);
	//tFinal = vec4(clamp(fTexBlendFactor.x, 0.0, 1.0)) + tFinal*0.001;

	// compute lighting
	//vec3 lightPoint = vec3(0.0, 30.0, 0.0);
	vec3 lightDir = normalize(vec3(2.0, -1.0, -0.9));
	//vec3 lightVec = fWPos - lightPoint;
	//float lightDist = length(lightVec);
	//vec3 lightDir = lightVec / lightDist;
	vec3 lightColor = normalize(vec3(1.0, 0.95, 0.9));
	float lightIntensity = 2.0;
	vec3 light = lightColor * lightIntensity * max(dot(-lightDir, normalize(fNormal)), 0.0);
	float falloff = 1.0; //1.0 / (lightDist*lightDist);
	vec3 ambientLight = vec3(0.01, 0.02, 0.05);
	
	vec3 totalLight = light * falloff + ambientLight;

	vec4 final = vec4(totalLight * (fColor * tFinal).xyz, 1.0);

	// DEBUG:
	//final = vec4(totalLight, 1.0) + 0.01 * final;
	//final = vec4(tFinal.xyz, 1.0) + 0.001 * final;

	gl_FragColor = final;
}
