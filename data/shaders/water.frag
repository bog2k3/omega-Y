#version 330 core

in vec3 fWPos;
in vec3 fNormal;
in vec2 fUV;
in float fFog;
in vec3 fScreenUV;

uniform float time;
uniform float screenAspectRatio;
uniform vec3 eyePos;
uniform mat4 mPV;
uniform sampler2D textureNormal;
uniform sampler2D textureReflection;
uniform sampler2D textureRefraction;

float fresnel(vec3 normal, vec3 incident, float n1, float n2) {
	// original fresnel code:
/*
	float r0 = (n1-n2) / (n1+n2);
	r0 *= r0;
	float cosX = -dot(normal, incident);
	if (n1 > n2)
	{
		float n = n1/n2;
		float sinT2 = n*n*(1.0-cosX*cosX);
		// Total internal reflection
		if (sinT2 > 1.0f)
			return 1.0f;
		cosX = sqrt(1.0f-sinT2);
	}
	float x = 1.0f - cosX;
	float fres = r0 + (1.0f-r0) * pow(x, 5);
	return fres;
*/
	// code adapted without branching:
	float r0 = (n1-n2) / (n1+n2);
	r0 *= r0;
	float cosX = -dot(normal, incident);
	float n = n1/n2;
	float sinT2 = n*n*(1.0-cosX*cosX);
	float cosXN1gtN2 = sqrt(1.0f-min(1.0, sinT2));
	cosX = mix(cosX, cosXN1gtN2, sign(n1 - n2) * 0.5 + 0.5);
	float x = 1.0f - cosX;
	float fres = r0 + (1.0f-r0) * pow(x, 5);
	return max(0.0, min(1.0, fres));
}

const float nAir = 1.0;
const float nWater = 1.33;
const float Zn = 0.15;
const float Zf = 500.0;
const float fov = 3.1415/2.5; // vertical field of view

vec3 underToAboveTransm(vec3 normal, vec2 screenCoord, float Zn, float Zf, float dxyW, vec3 eyeDir, float eyeDist) {
	const vec3 smoothNormal = vec3(0, 1, 0);
	vec4 refractTarget = texture(textureRefraction, screenCoord);
	float targetZ = Zn + (Zf - Zn) * refractTarget.a;
	float targetDist = sqrt(targetZ*targetZ * (1 + dxyW*dxyW / (Zn*Zn)));
	float targetDistUW = targetDist - eyeDist; // distance through water to target 0
	//float targetElevation = targetDistUW * sqrt(1.0 - pow(dot(eyeDir, smoothNormal), 2));

	vec3 T = refract(-eyeDir, normal, 1.0 / nWater);
	//float T_targetDist = targetDistUW * dot(T, -smoothNormal);//(targetDist - eyeDist) // distance through water to refracted target
	//float T_targetElevation = T_targetDist * sqrt(1.0 - pow(dot(eyeDir, smoothNormal), 2));
	//vec3 wPosT = fWPos + T * T_targetDist;
	//vec4 projT = mPV * vec4(wPosT, 1.0);
	//projT.xyz /= projT.w;
	//float assumed_T_dist = length(wPosT - eyePos);

	//vec2 transmitCoord = projT.xy * 0.5 + 0.5;//screenCoord;
	//vec4 transmitColor = texture(textureRefraction, transmitCoord);
	//float transmitZ = Zn + (Zf - Zn) * transmitColor.a;
	//float actual_T_dist = sqrt(transmitZ*transmitZ * (1 + dxyW*dxyW / (Zn*Zn)));
	//float transmitElevation = - wPosT.y;
	//float transmitAttenuation = 0;//clamp((assumed_T_dist - actual_T_dist) / assumed_T_dist, 0, 1);
	//transmitAttenuation = pow(transmitAttenuation, 2);
	//transmitCoord = mix(transmitCoord, screenCoord, transmitAttenuation);
	//transmitColor = mix(transmitColor, refractTarget, transmitAttenuation);
	//transmitColor = texture(textureRefraction, transmitCoord);
	//transmitColor = refractTarget;

	float thicknessRefFactor = pow(abs(targetDistUW) / 2.5, 0.6);
	float distanceRefractionFactor = 1.0 / (1 + pow(eyeDist, 0.5));
	vec3 w_perturbation = (smoothNormal - normal) * 1.0;
	vec2 s_perturbation = (mPV * vec4(w_perturbation, 0)).xz;
	vec2 sampleCoord = screenCoord + s_perturbation * thicknessRefFactor * distanceRefractionFactor;
	vec4 transmitColor = texture(textureRefraction, sampleCoord);

	float fresnelFactor = 1 - fresnel(normal, -T, nWater, nAir);

	//return vec3(perturbation.x, perturbation.z, 0);

	return transmitColor.xyz * fresnelFactor;
}

vec3 aboveToUnderTransm(vec3 normal, vec2 screenCoord, float Zn, float Zf, float dxyW, vec3 eyeDir, float eyeDist) {
	const vec3 smoothNormal = vec3(0, -1, 0);
	vec4 refractTarget = texture(textureRefraction, screenCoord);
	float targetZ = Zn + (Zf - Zn) * refractTarget.a;
	float targetDist = sqrt(targetZ*targetZ * (1 + dxyW*dxyW / (Zn*Zn)));
	float targetDistUW = targetDist - eyeDist; // distance through water to target 0
	//float targetElevation = targetDistUW * sqrt(1.0 - pow(dot(eyeDir, smoothNormal), 2));

	vec3 T = refract(-eyeDir, -normal, nWater);
	float T_targetDist = targetDistUW * dot(T, -smoothNormal);//(targetDist - eyeDist) // distance through water to refracted target
	float T_targetElevation = T_targetDist * sqrt(1.0 - pow(dot(eyeDir, smoothNormal), 2));
	vec3 wPosT = fWPos + T * T_targetDist;
	vec4 projT = mPV * vec4(wPosT, 1.0);
	projT.xyz /= projT.w;
	float assumed_T_dist = length(wPosT - eyePos);

	vec2 transmitCoord = projT.xy * 0.5 + 0.5;//screenCoord;
	vec4 transmitColor = texture(textureRefraction, transmitCoord);
	float transmitZ = Zn + (Zf - Zn) * transmitColor.a;
	float actual_T_dist = sqrt(transmitZ*transmitZ * (1 + dxyW*dxyW / (Zn*Zn)));
	//float transmitElevation = - wPosT.y;
	float transmitAttenuation = 0;//clamp((assumed_T_dist - actual_T_dist) / assumed_T_dist, 0, 1);
	//transmitAttenuation = pow(transmitAttenuation, 2);
	//transmitCoord = mix(transmitCoord, screenCoord, transmitAttenuation);
	//transmitColor = mix(transmitColor, refractTarget, transmitAttenuation);
	transmitColor = texture(textureRefraction, transmitCoord);
	//transmitColor = refractTarget;

	float fresnelFactor = 1 - fresnel(normal, -T, nAir, nWater);

	return transmitColor.xyz * fresnelFactor;
}

const vec3 smoothNormal = vec3(0, 1, 0);

vec3 normalPerturbation(float time) {
	const vec2 moveSpeed1 = vec2(0.02);

	vec2 modUV1 = fUV + time * moveSpeed1;
	vec3 texNormal1 = texture(textureNormal, modUV1 * 0.5).rbg * 2 - 1;
	float perturbStrength1 = 0.005;
	vec3 perturb1 = (texNormal1 - smoothNormal) * perturbStrength1;

	vec2 modUV2 = fUV.yx - time * moveSpeed1;
	vec3 texNormal2 = texture(textureNormal, modUV2 * 0.5).rbg * 2 - 1;
	vec3 perturb2 = (texNormal2 - smoothNormal) * perturbStrength1;

	float moveSpeed3 = -0.012;
	vec2 modUV3 = fUV + time * moveSpeed3;
	vec3 texNormal3 = texture(textureNormal, modUV3 * 2).rbg * 2 - 1;
	float perturbStrength3 = 0.03;
	vec3 perturb3 = (texNormal3 - smoothNormal) * perturbStrength3;

	vec2 modUV4 = fUV.yx - time * moveSpeed3;
	vec3 texNormal4 = texture(textureNormal, modUV4 * 2).rbg * 2 - 1;
	float perturbStrength4 = 0.07;
	vec3 perturb4 = (texNormal4 - smoothNormal) * perturbStrength4;

	vec2 modUV5 = fUV + time * moveSpeed3 * 0.5;
	vec3 texNormal5 = texture(textureNormal, modUV5 * 7).rbg * 2 - 1;
	vec3 perturb5 = (texNormal5 - smoothNormal) * perturbStrength4;

	return perturb1 + perturb2 + perturb3 + perturb4 + perturb5;
}

void main() {
	vec3 eyeDir = eyePos - fWPos;
	float eyeDist = length(eyeDir);
	eyeDir /= eyeDist; // normalize

// normal:
	vec3 perturbation = normalPerturbation(time * 1.0);
	float perturbationDistanceFactor = pow(min(1.0, 30.0 / (eyeDist+1)), 1.0);
	perturbation *= perturbationDistanceFactor;
	vec3 normal = normalize(smoothNormal + perturbation);

// other common vars:
	bool isCameraUnderWater = eyePos.y < 0;
	vec2 screenCoord = fScreenUV.xy / fScreenUV.z * 0.5 + 0.5;
	//float targetElevation = (texture(textureRefraction, screenCoord).a - 0.5) * 5;
	float dxy = length((screenCoord * 2 - 1) * vec2(screenAspectRatio, 1.0)); // screen-space distance from center
	float dxyW = dxy * Zn * tan(fov*0.5);// world-space distance from screen center at near-z plane

	vec3 transmitColor = isCameraUnderWater
		? aboveToUnderTransm(normal, screenCoord, Zn, Zf, dxyW, eyeDir, eyeDist)
		: underToAboveTransm(normal, screenCoord, Zn, Zf, dxyW, eyeDir, eyeDist);

// compute reflection
	float distanceReflectionFactor = min(1.0, 1.5 / (1 + pow(eyeDist, 0.14)));
	distanceReflectionFactor *= 0.5;
	vec2 s_perturb = (mPV * vec4(perturbation, 0)).xz;
	vec2 reflectCoord = vec2(1 - screenCoord.x, screenCoord.y) + vec2(-s_perturb.x, s_perturb.y) * distanceReflectionFactor;

	//vec2 reflectCoord = vec2(1-screenCoord.x, screenCoord.y);
	vec4 reflectColor = texture(textureReflection, reflectCoord);
	float reflectFresnelFactor = fresnel(normal, -eyeDir, isCameraUnderWater ? nWater : nAir, isCameraUnderWater ? nAir : nWater);
	reflectColor.xyz *= reflectFresnelFactor;

	vec3 reflectTint = vec3(0.5, 0.6, 0.65) * 1.3;
	//reflectColor.xyz *= reflectTint;
	//reflectColor = vec4(0);

// mix reflection and refraction:
	vec4 final = vec4( transmitColor + reflectColor.xyz, 1.0);

// fade out far edges of water
	//float targetElevationNormalized = targetElevation / 2.5;
	//float elevationAlphaFactor = min(1.0, pow(abs(targetElevationNormalized) * 10, 3));
	float alpha = /*elevationAlphaFactor */ (1-pow(fFog, 3.0));
	//final.a = alpha;

// DEBUG:
	//float f = pow(abs((T_targetElevation - transmitElevation) / T_targetElevation), 2.2);
	//float f = pow(fresnelFactor, 2.2);
	//final = vec4(f, f, f, 1.0) + 0.00001 * final;
	//final = vec4(transmitColor.xyz, 1.0) + 0.00001 * final;
	//final.a = 0.00001;

	gl_FragColor = final;
}
