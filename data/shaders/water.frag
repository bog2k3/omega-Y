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

const float nAir = 1.0;
const float nWater = 1.33;
const float Zn = 0.15;
const float Zf = 500.0;
const float fov = 3.1415/2.5; // vertical field of view

const vec3 smoothNormal = vec3(0, 1, 0);

vec3 underToAboveTransm(vec3 normal, vec2 screenCoord, float dxyW, vec3 eyeDir, float eyeDist) {
	vec4 refractTarget = texture(textureRefraction, screenCoord);
	float targetZ = Zn + (Zf - Zn) * refractTarget.a;
	float targetDist = sqrt(targetZ*targetZ * (1 + dxyW*dxyW / (Zn*Zn)));
	float targetDistUW = targetDist - eyeDist; // distance through water to target 0

	vec3 T = refract(-eyeDir, normal, 1.0 / nWater);
	float targetDepth = targetDistUW * dot(T, -smoothNormal);
	float t_t0 = acos(dot(-T, normal)) - acos(dot(-T, smoothNormal));
	float displacement = Zn * (targetDist - eyeDist) * tan(t_t0) / targetDist;

	vec3 w_perturbation = (normal-smoothNormal) * displacement * 40;
	vec2 s_perturbation = (mPV * vec4(w_perturbation, 0)).xy;
	s_perturbation *= pow(clamp(targetDepth*0.4, 0, 1), 1);
	vec2 sampleCoord = screenCoord + s_perturbation;
	vec4 transmitColor = texture(textureRefraction, sampleCoord);

	float fresnelFactor = 1 - fresnel(normal, -T, nWater, nAir);

	return transmitColor.xyz * fresnelFactor;
}

vec3 aboveToUnderTransm(vec3 normal, vec2 screenCoord, float dxyW, vec3 eyeDir, float eyeDist) {
	vec3 T = refract(-eyeDir, -normal, nWater);	
	vec4 refractTarget = texture(textureRefractionCube, T);

	float fresnelFactor = 1 - fresnel(normal, -T, nAir, nWater);

	return refractTarget.xyz * fresnelFactor;
}

vec4 reflection(vec3 normal, vec2 screenCoord, float dxyW, vec3 eyeDir, float eyeDist, float distortionAmp) {
	vec4 reflectTarget = texture(textureReflection2D, vec2(1 - screenCoord.x, screenCoord.y));
	float targetZ = Zn + (Zf - Zn) * reflectTarget.a;
	float targetDist = sqrt(targetZ*targetZ * (1 + dxyW*dxyW / (Zn*Zn)));
	
	float r_r0 = acos(dot(-eyeDir, normal)) - acos(dot(-eyeDir, smoothNormal));
	float displacement = Zn * (targetDist - eyeDist) * tan(r_r0) / targetDist;
	vec2 s_perturb = (mPV * vec4(normal - smoothNormal, 0)).xy * displacement * 30 * distortionAmp;
	vec2 reflectCoord = vec2(1 - screenCoord.x, screenCoord.y) + s_perturb;

	vec4 reflectColor = texture(textureReflection2D, reflectCoord);
	
	return reflectColor;
}

// compute reflection above water surface
vec3 aboveReflection(vec3 normal, vec2 screenCoord, float dxyW, vec3 eyeDir, float eyeDist) {
	vec4 reflectColor = reflection(normal, screenCoord, dxyW, eyeDir, eyeDist, 1);
	float reflectFresnelFactor = fresnel(normal, -eyeDir, nAir, nWater);
	reflectColor.xyz *= reflectFresnelFactor;
	
	return reflectColor.xyz;
}

// compute reflection below water surface
vec3 belowReflection(vec3 normal, vec2 screenCoord, float dxyW, vec3 eyeDir, float eyeDist) {
	vec4 reflectColor = reflection(normal, screenCoord, dxyW, eyeDir, eyeDist, 5);
	float reflectFresnelFactor = fresnel(-normal, -eyeDir, nWater, nAir);
	reflectColor.xyz *= reflectFresnelFactor;
	
	return reflectColor.xyz;
}

vec3 computeNormal(float time, float eyeDist) {
	const vec2 moveSpeed1 = vec2(0.02, 0.1) * 0.15;
	const float density1 = 1.0;
	const float amplitude1 = 0.2;
	float amplitudeDistanceFactor1 = pow(min(1.0, 30.0 / (eyeDist+1)), 1.0);
	vec2 uv1 = (fUV + (time * moveSpeed1)) * density1;
	vec3 texNormal1 = (texture(textureNormal, uv1 * 0.5).rbg * 2 - 1) * amplitude1 * amplitudeDistanceFactor1;

	const vec2 moveSpeed2 = -vec2(0.02, 0.1) * 0.12;
	const float density2 = 10.0;
	const float amplitude2 = 0.2;
	float amplitudeDistanceFactor2 = pow(min(1.0, 30.0 / (eyeDist+1)), 1.0);
	vec2 uv2 = (fUV + (time * moveSpeed2)) * density2;
	vec3 texNormal2 = (texture(textureNormal, uv2 * 0.5).rbg * 2 - 1) * amplitude2 * amplitudeDistanceFactor2;

	const vec2 moveSpeed3 = -vec2(0.02, 0.1) * 0.05;
	const float density3 = 25.0;
	const float amplitude3 = 0.2;
	float amplitudeDistanceFactor3 = pow(min(1.0, 30.0 / (eyeDist+1)), 1.0);
	vec2 uv3 = (fUV + (time * moveSpeed3)) * density3;
	vec3 texNormal3 = (texture(textureNormal, uv3 * 0.5).rbg * 2 - 1) * amplitude3 * amplitudeDistanceFactor3;

	const vec2 moveSpeed4 = -moveSpeed3;
	const float density4 = density3;
	const float amplitude4 = amplitude3;
	float amplitudeDistanceFactor4 = amplitudeDistanceFactor3;
	vec2 uv4 = (fUV + (time * moveSpeed4)) * density4;
	vec3 texNormal4 = (texture(textureNormal, uv4 * 0.5).rbg * 2 - 1) * amplitude4 * amplitudeDistanceFactor4;

	vec3 final = normalize(smoothNormal + texNormal1 + texNormal2 + texNormal3 + texNormal4);

	return final;
}

void main() {
	vec3 eyeDir = eyePos - fWPos;
	float eyeDist = length(eyeDir);
	eyeDir /= eyeDist; // normalize

// normal:
	vec3 normal = computeNormal(time * 1.0, eyeDist);
	//normal = smoothNormal;

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

// fade out far edges of water
	//float targetElevationNormalized = targetElevation / 2.5;
	//float elevationAlphaFactor = min(1.0, pow(abs(targetElevationNormalized) * 1, 3));
	/*vec4 refractTarget = texture(textureRefraction, screenCoord);
	float targetZ = Zn + (Zf - Zn) * refractTarget.a;
	float targetDist = sqrt(targetZ*targetZ * (1 + dxyW*dxyW / (Zn*Zn)));
	float targetDistUW = targetDist - eyeDist + 0.1;
	float depthAlphaFactor = clamp(pow(targetDistUW * 10, 1), 0, 1);*/

	float alpha = /*depthAlphaFactor * */ (1-pow(fFog, 3.0));
	final.a = alpha;

// DEBUG:
	//float f = pow(abs((T_targetElevation - transmitElevation) / T_targetElevation), 2.2);
	//float f = pow(displacement*20, 2.2);
	//final = vec4(f, f, f, 1.0) + 0.00001 * final;
	//final = vec4(distanceReflectionFactor.xy, 0, 1.0) + 0.00001 * final;
	//final.a = 0.00001;

	gl_FragColor = final;
}
