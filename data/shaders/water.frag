#version 330 core

in vec3 fWPos;
in vec3 fNormal;
in vec2 fUV;
in float fFog;

uniform float time;
uniform vec3 eyePos;
uniform sampler2D textureDuDv;
uniform samplerCube textureReflection;

void main() {
	vec3 normal = vec3(0.0, 1.0, 0.0);

	float changeSpeed = 0.02;
	float change = time * changeSpeed;

	float perturbFreq1 = 0.5;
	float perturbStrength1 = 0.1;
	vec4 dudv = texture(textureDuDv, (fUV + change) * perturbFreq1) * 2.0 - 1.0;
	vec3 perturb1 = dudv.rbg * perturbStrength1;

	float perturbFreq2 = 2.0;
	float perturbStrength2 = 0.05;
	dudv = texture(textureDuDv, (fUV - change) * perturbFreq2) * 2.0 - 1.0;
	vec3 perturb2 = dudv.rbg * perturbStrength2;

	float perturbFreq3 = 8.0;
	float perturbStrength3 = 0.03;
	dudv = texture(textureDuDv, (fUV.yx + change) * perturbFreq3) * 2.0 - 1.0;
	vec3 perturb3 = dudv.rbg * perturbStrength3;

	float perturbTotalFactor = (1.0 - pow(fFog, 0.8)) * 0.5;
	vec3 perturbTotal = perturbTotalFactor * (perturb1 + perturb2 + perturb3);
	perturbTotal.y = 0;
	normal = normalize(normal + perturbTotal);

	vec3 eyeDir = normalize(eyePos - fWPos);

	vec3 reflectDir = reflect(-eyeDir, normal);
	vec4 reflectColor = texture(textureReflection, reflectDir);

	vec3 fogDir = -eyeDir;
	fogDir.y = 0;
	vec4 fogColor = texture(textureReflection, fogDir);
	float fogFactor = pow(fFog, 2.0);

	vec4 final = vec4(mix(reflectColor.xyz, fogColor.xyz, fogFactor), 1.0);

	// DEBUG:
	//final = vec4(fWPos + fNormal, 1.0) + fColor;
	//final.xy += fUV;
	//final = vec4(f, f, f, 1.0) + 0.00001 * final;

	gl_FragColor = final;
}
