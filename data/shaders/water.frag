#version 330 core

in vec3 fWPos;
in vec3 fNormal;
in vec2 fUV;
in float fFog;
in vec3 fScreenUV;

uniform float time;
uniform vec3 eyePos;
uniform sampler2D textureNormal;
uniform samplerCube textureReflection;
uniform sampler2D textureRefraction;

float fresnel(vec3 normal, vec3 incident) {
	float f = 1.0 - pow(dot(normal, -incident), 2.0);
	return max(0.05, min(1.0, pow(f, 10) * 1.3));
 }

void main() {
	vec3 eyeDir = eyePos - fWPos;
	float eyeDist = length(eyeDir);
	eyeDir /= eyeDist; // normalize

// compute normal perturbation
	vec3 smoothNormal = vec3(0, 1, 0);

	vec2 moveSpeed1 = vec2(0.02);
	vec2 modUV1 = fUV + time * moveSpeed1;
	vec3 texNormal1 = texture(textureNormal, modUV1 * 0.5).rbg * 2 - 1;
	float perturbStrength1 = 0.10;
	vec3 perturb1 = (texNormal1 - smoothNormal) * perturbStrength1;

	vec2 modUV2 = fUV.yx - time * moveSpeed1;
	vec3 texNormal2 = texture(textureNormal, modUV2 * 0.5).rbg * 2 - 1;
	vec3 perturb2 = (texNormal2 - smoothNormal) * perturbStrength1;

	float moveSpeed3 = -0.012;
	vec2 modUV3 = fUV + time * moveSpeed3;
	vec3 texNormal3 = texture(textureNormal, modUV3 * 2).rbg * 2 - 1;
	float perturbStrength3 = 0.1;
	vec3 perturb3 = (texNormal3 - smoothNormal) * perturbStrength3;

	vec2 modUV4 = fUV.yx - time * moveSpeed3;
	vec3 texNormal4 = texture(textureNormal, modUV4 * 2).rbg * 2 - 1;
	vec3 perturb4 = (texNormal4 - smoothNormal) * perturbStrength3;

	vec2 modUV5 = fUV + time * moveSpeed3 * 0.5;
	vec3 texNormal5 = texture(textureNormal, modUV5 * 7).rbg * 2 - 1;
	vec3 perturb5 = (texNormal5 - smoothNormal) * perturbStrength3;

	vec3 perturbation = perturb1 + perturb2 + perturb3 + perturb4 + perturb5;
	float perturbationDistanceFactor = pow(min(1.0, 30.0 / (eyeDist+1)), 1.0);
	vec3 normal = normalize(smoothNormal + perturbation * perturbationDistanceFactor);

// compute reflection
	vec3 reflectDir = reflect(-eyeDir, normal);
	vec4 reflectColor = texture(textureReflection, reflectDir);

	vec3 reflectTint = vec3(0.5, 0.6, 0.65) * 1.3;
	reflectColor.xyz *= reflectTint;

// compute refraction
	vec2 refractCoord = fScreenUV.xy / fScreenUV.z * 0.5 + 0.5;
	float targetElevation = texture(textureRefraction, refractCoord).a;
	float elevationRefFactor = pow(abs(targetElevation - 0.5) * 2, 3);
	refractCoord += perturbation.xz * elevationRefFactor / pow(eyeDist, 0.8);
	vec4 refractColor = texture(textureRefraction, refractCoord);

// mix reflection and refraction:
	float fresnelFactor = fresnel(normal, -eyeDir);
	vec4 final = vec4(mix(refractColor.xyz, reflectColor.xyz, fresnelFactor), 1.0);

// fade out far edges of water
	float alpha = pow(elevationRefFactor, 0.5);
	alpha *= (1-pow(fFog, 3.0));
	final.a = alpha;

// DEBUG:
	float f = elevationRefFactor;
	//final = vec4(f, f, f, 1.0) + 0.00001 * final;
	//final = vec4(normal.xyz, 1.0) + 0.00001 * final;
	//final.a = 0.00001;

	gl_FragColor = final;
}
