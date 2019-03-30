#version 330 core

in vec3 fWPos;
in vec3 fNormal;
in vec4 fColor;
in vec2 fUV[5];
in vec4 fTexBlendFactor;

uniform vec3 eyePos;
uniform int bReflection;

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
	vec4 t01 = mix(t0, t1, clamp(fTexBlendFactor.x, 0.0, 1.0)); // dirt and grass
	vec4 t23 = mix(t2, t3, clamp(fTexBlendFactor.y, 0.0, 1.0)); // rock 1 and rock 2
	vec4 tGrassOrSand = vec4(mix(t01, t4, clamp(fTexBlendFactor.w, 0.0, 1.0)).xyz, 1.0); // grass/dirt and sand
	vec4 tFinal = vec4(mix(tGrassOrSand, t23, 1.0 - clamp(fTexBlendFactor.z, 0.0, 1.0)).xyz, 1.0);
	//tFinal = vec4(clamp(fTexBlendFactor.x, 0.0, 1.0)) + tFinal*0.001;

	// compute lighting
	//vec3 lightPoint = vec3(0.0, 30.0, 0.0);
	vec3 lightDir = normalize(vec3(2.0, -1.0, -0.9));
	//vec3 lightVec = fWPos - lightPoint;
	//float lightDist = length(lightVec);
	//vec3 lightDir = lightVec / lightDist;
	vec3 lightColor = normalize(vec3(1.0, 0.95, 0.9));
	float lightIntensity = 2.0;
	lightColor *= lightIntensity;

	vec3 eyeDir = eyePos - fWPos;
	float eyeDist = length(eyeDir);
	eyeDir /= eyeDist;
	float eyeHeight = eyePos.y * (bReflection > 0 ? -1 : +1);

	// for underwater terrain, we need to simulate light absorbtion through water
	float waterLevel = 0;
	vec3 lightHalveDist = vec3(2.0, 3.0, 4.0) * 1.5; // after how many meters of water each light component is halved
	float lightWaterDistance = fWPos.y / lightDir.y;
	lightWaterDistance += eyeHeight < 0 ? eyeDist : 0;
	vec3 absorbFactor = 1.0 / pow(vec3(2.0), vec3(lightWaterDistance) / lightHalveDist);
	absorbFactor = fWPos.y < waterLevel ? absorbFactor : vec3(1.0);
	lightColor *= absorbFactor;

	vec3 light = lightColor * max(dot(-lightDir, normalize(fNormal)), 0.0);

	vec3 waterColor = vec3(0.03, 0.08, 0.1) * lightIntensity;
	vec3 ambientLightAbove = vec3(0.01, 0.02, 0.05) * lightIntensity;
	vec3 ambientLightBelow = waterColor / pow(max(1, 1 - fWPos.y), 1.5);
	float ambientMixDistance = 1;
	float ambientMixFactor = clamp(fWPos.y + ambientMixDistance, 0.0, 1.0);
	vec3 ambientLight = mix(ambientLightBelow, ambientLightAbove, ambientMixFactor);

	vec3 totalLight = light + ambientLight;

	vec4 final = vec4(totalLight * (fColor * tFinal).xyz, 1.0);
	float Zn = 0.15;
	float Zf = 500.0;
	final.a = clamp((gl_FragCoord.z / gl_FragCoord.w - Zn) / (Zf - Zn), 0.0, 1.0); // fragment's z

	// water fog:
	vec3 waterNormal = vec3(0.0, 1.0, 0.0);
	float waterThickness = eyeDist * (eyeHeight < 0 ? 1 : -fWPos.y / (eyeHeight - fWPos.y));
	//waterThickness *= 0.2;
	float fogFactor = clamp(1.0 - 1.0 / (waterThickness * 0.05 + 1), 0, 1);
	float depthFactor = pow(1.0 / (max(0, -eyePos.y) + 1), 0.5);
	fogFactor *= fWPos.y < waterLevel ? 1.0 : 0.0;
	final.xyz = mix(final.xyz, waterColor * depthFactor, fogFactor);

	// DEBUG:
	//final = vec4(absorption, 1.0) + 0.01 * final;
	float f = waterThickness / 20;
	//f = eyeDist / 50;
	//final = vec4(f, f, f, 1.0) + 0.00001 * final;
	//final.xyz = tFinal.xyz + 0.00001 * final.xyz;

	gl_FragColor = final;
}
