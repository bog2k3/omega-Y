#version 330 core

#include underwater.glsl

in vec3 fWPos;
in vec3 fNormal;
in float fFog;
in vec3 fScreenUV;

uniform float time;
uniform float screenAspectRatio;
uniform vec3 eyePos;
uniform mat4 mPV;
uniform sampler2D textureReflection2D;
uniform sampler2D textureRefraction;
uniform samplerCube textureRefractionCube;

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

vec3 underToAboveTransm(vec3 normal, vec2 screenCoord, float dxyW, vec3 eyeDir, float eyeDist) {
	vec4 refractTarget = texture(textureRefraction, screenCoord);
	float targetZ = Zn + (Zf - Zn) * refractTarget.a;
	float targetDist = sqrt(targetZ*targetZ * (1 + dxyW*dxyW / (Zn*Zn)));
	float targetDistUW = targetDist - eyeDist; // distance through water to target 0

	vec3 T = refract(-eyeDir, normal, 1.0 / n_water);
	float targetDepth = targetDistUW * dot(T, -waterSmoothNormal);
	float t_t0 = acos(dot(-T, normal)) - acos(dot(-T, waterSmoothNormal));
	float displacement = Zn * (targetDist - eyeDist) * tan(t_t0) / targetDist;

	vec3 w_perturbation = (normal-waterSmoothNormal) * displacement * 40;
	vec2 s_perturbation = (mPV * vec4(w_perturbation, 0)).xy;
	s_perturbation *= pow(clamp(targetDepth*0.4, 0, 1), 1);
	vec2 sampleCoord = screenCoord + s_perturbation;
	vec4 transmitColor = texture(textureRefraction, sampleCoord);

	float fresnelFactor = 1 - fresnel(normal, -T, n_water, n_air);

	return transmitColor.xyz * fresnelFactor;
}

vec3 aboveToUnderTransm(vec3 normal, vec2 screenCoord, float dxyW, vec3 eyeDir, float eyeDist) {
	vec3 T = refract(-eyeDir, -normal, n_water);
	vec4 refractTarget = texture(textureRefractionCube, T);

	float fresnelFactor = 1 - fresnel(normal, -T, n_air, n_water);

	vec3 refractColor = refractTarget.rgb * fresnelFactor;

	// simulate light absorbtion through water
	vec3 lightHalveDist = vec3(2.0, 3.0, 4.0) * 1.5; // after how many meters of water each light component is halved
	vec3 absorbFactor = 1.0 / pow(vec3(2.0), vec3(eyeDist) / lightHalveDist);
	refractColor *= absorbFactor;

	return refractColor;
}

vec4 reflection(vec3 normal, vec2 screenCoord, float dxyW, vec3 eyeDir, float eyeDist) {
	vec4 reflectTarget = texture(textureReflection2D, vec2(1 - screenCoord.x, screenCoord.y));
	float targetZ = Zn + (Zf - Zn) * reflectTarget.a;
	float targetDist = sqrt(targetZ*targetZ * (1 + dxyW*dxyW / (Zn*Zn)));

	float r_r0 = acos(dot(-eyeDir, normal)) - acos(dot(-eyeDir, waterSmoothNormal));
	float displacement = Zn * (targetDist - eyeDist) / targetDist * tan(r_r0);
	vec2 s_perturb = (mPV * vec4(normal - waterSmoothNormal, 0)).xy * displacement * 30;
	vec2 reflectCoord = vec2(1 - screenCoord.x, screenCoord.y) + s_perturb;

	vec4 reflectColor = texture(textureReflection2D, reflectCoord);

	//reflectColor = vec4(s_perturb*30, 0, 1);
	//reflectColor = vec4(vec3(targetZ), 1);

	return reflectColor;
}

// compute reflection above water surface
vec3 aboveReflection(vec3 normal, vec2 screenCoord, float dxyW, vec3 eyeDir, float eyeDist) {
	vec4 reflectColor = reflection(normal, screenCoord, dxyW, eyeDir, eyeDist);
	float reflectFresnelFactor = fresnel(normal, -eyeDir, n_air, n_water);
	reflectColor.xyz *= reflectFresnelFactor;

	return reflectColor.xyz;
}

// compute reflection below water surface
vec3 belowReflection(vec3 normal, vec2 screenCoord, float dxyW, vec3 eyeDir, float eyeDist) {
	vec4 reflectColor = reflection(normal, screenCoord, dxyW, eyeDir, eyeDist);
	float reflectFresnelFactor = fresnel(-normal, -eyeDir, n_water, n_air);
	reflectColor.xyz *= reflectFresnelFactor;

	return reflectColor.xyz;
}

void main() {
	vec3 eyeDir = eyePos - fWPos;
	float eyeDist = length(eyeDir);
	eyeDir /= eyeDist; // normalize
	float angleNormalFactor = 1; //pow(abs(dot(eyeDir, waterSmoothNormal)), 0.9);

// normal:
	float perturbAmplitude = angleNormalFactor * (eyePos.y > 0 ? 1 : 2);
	vec3 normal = computeWaterNormal(fWPos.xz, time * 1.0, eyeDist, perturbAmplitude);
	//normal = waterSmoothNormal;

// other common vars:
	bool isCameraUnderWater = eyePos.y < 0;
	vec2 screenCoord = fScreenUV.xy / fScreenUV.z * 0.5 + 0.5;
	float dxy = length((screenCoord * 2 - 1) * vec2(screenAspectRatio, 1.0)); // screen-space distance from center
	float dxyW = dxy * Zn * tan(fov*0.5);// world-space distance from screen center at near-z plane

	vec3 transmitColor = isCameraUnderWater
		? aboveToUnderTransm(normal, screenCoord, dxyW, eyeDir, eyeDist)
		: underToAboveTransm(normal, screenCoord, dxyW, eyeDir, eyeDist);
	//transmitColor = vec3(0);

// compute reflection
	vec3 reflectColor = isCameraUnderWater
		? belowReflection(normal, screenCoord, dxyW, eyeDir, eyeDist)
		: aboveReflection(normal, screenCoord, dxyW, eyeDir, eyeDist);
	//reflectColor = vec3(0);

	//vec3 reflectTint = normalize(vec3(0.55, 0.6, 0.65)) * 1.5;
	//reflectColor.xyz *= reflectTint;

// mix reflection and refraction:
	vec4 final = vec4( transmitColor + reflectColor.xyz, 1.0);

	const float lightIntensity = 2.0;
	const vec3 waterColor = vec3(0.03, 0.08, 0.1) * lightIntensity;
	float fogFactor = isCameraUnderWater ? clamp(1.0 - 1.0 / (eyeDist * 0.2 + 1), 0, 1) : 0.0;
	float depthFactor = pow(1.0 / (max(0, -eyePos.y) + 1), 0.5);
	final.rgb = mix(final.rgb, waterColor * depthFactor, fogFactor);

// fade out far edges of water
	//float targetElevationNormalized = targetElevation / 2.5;
	//float elevationAlphaFactor = min(1.0, pow(abs(targetElevationNormalized) * 1, 3));
	/*vec4 refractTarget = texture(textureRefraction, screenCoord);
	float targetZ = Zn + (Zf - Zn) * refractTarget.a;
	float targetDist = sqrt(targetZ*targetZ * (1 + dxyW*dxyW / (Zn*Zn)));
	float targetDistUW = targetDist - eyeDist + 0.1;
	float depthAlphaFactor = clamp(pow(targetDistUW * 10, 1), 0, 1);*/

	float alpha = /*depthAlphaFactor * */ (1-pow(fFog, 3.0));
	final.a = !isCameraUnderWater ? alpha : 1.0;

	//final.xyz = reflectColor;

// DEBUG:
	//float f = pow(abs((T_targetElevation - transmitElevation) / T_targetElevation), 2.2);
	//float f = pow(displacement*20, 2.2);
	//final = vec4(f, f, f, 1.0) + 0.00001 * final;
	//final = vec4(reflectColor.rgb, 1.0) + 0.00001 * final;
	//final.a = 0.00001;

	gl_FragColor = final;
}
