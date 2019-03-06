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
	float time_ = 1 * time;

	vec2 moveSpeed1 = vec2(0.02);
	vec2 modUV1 = fUV + time_ * moveSpeed1;
	vec3 texNormal1 = texture(textureNormal, modUV1 * 0.5).rbg * 2 - 1;
	float perturbStrength1 = 0.005;
	vec3 perturb1 = (texNormal1 - smoothNormal) * perturbStrength1;

	vec2 modUV2 = fUV.yx - time_ * moveSpeed1;
	vec3 texNormal2 = texture(textureNormal, modUV2 * 0.5).rbg * 2 - 1;
	vec3 perturb2 = (texNormal2 - smoothNormal) * perturbStrength1;

	float moveSpeed3 = -0.012;
	vec2 modUV3 = fUV + time_ * moveSpeed3;
	vec3 texNormal3 = texture(textureNormal, modUV3 * 2).rbg * 2 - 1;
	float perturbStrength3 = 0.03;
	vec3 perturb3 = (texNormal3 - smoothNormal) * perturbStrength3;

	vec2 modUV4 = fUV.yx - time_ * moveSpeed3;
	vec3 texNormal4 = texture(textureNormal, modUV4 * 2).rbg * 2 - 1;
	float perturbStrength4 = 0.07;
	vec3 perturb4 = (texNormal4 - smoothNormal) * perturbStrength4;

	vec2 modUV5 = fUV + time_ * moveSpeed3 * 0.5;
	vec3 texNormal5 = texture(textureNormal, modUV5 * 7).rbg * 2 - 1;
	vec3 perturb5 = (texNormal5 - smoothNormal) * perturbStrength4;

	vec3 perturbation = perturb1 + perturb2 + perturb3 + perturb4 + perturb5;
	float perturbationDistanceFactor = pow(min(1.0, 30.0 / (eyeDist+1)), 1.0);
	vec3 normal = normalize(smoothNormal + perturbation * perturbationDistanceFactor);

	vec2 screenCoord = fScreenUV.xy / fScreenUV.z * 0.5 + 0.5;
	//float targetElevation = (texture(textureRefraction, screenCoord).a - 0.5) * 5;	
	float Zn = 0.15;
	float Zf = 500.0;
	float dxy = length((screenCoord * 2 - 1) * vec2(screenAspectRatio, 1.0)); // screen-space distance from center
	float fov = 3.1415/2.5; // vertical field of view
	float dxyW = dxy * Zn * tan(fov*0.5);// world-space distance from screen center at near-z plane
	
	vec4 refractTarget = texture(textureRefraction, screenCoord);
	float targetZ = Zn + (Zf - Zn) * refractTarget.a;
	float targetDist = sqrt(targetZ*targetZ * (1 + dxyW*dxyW / (Zn*Zn)));

// compute reflection
	float distanceReflectionFactor = min(1.0, 1.5 / (1 + pow(eyeDist, 0.4)));
	float elevationReflectionFactor = 1.0;//min(abs(targetElevation), 1.0);
	float reflectionPerturbFactor = distanceReflectionFactor * elevationReflectionFactor;
	vec2 reflectCoord = vec2(1-screenCoord.x, screenCoord.y) + perturbation.xz * reflectionPerturbFactor;

	//vec2 reflectCoord = vec2(1-screenCoord.x, screenCoord.y);
	vec4 reflectColor = texture(textureReflection, reflectCoord);

	vec3 reflectTint = vec3(0.5, 0.6, 0.65) * 1.3;
	reflectColor.xyz *= reflectTint;

// compute refraction
	/*float elevationRefFactor = pow(abs(targetElevation) / 2.5, 0.6);
	float elevationRefractionFactor = 1.0 / (1 + pow(eyeDist, 0.8));
	vec2 transmitCoord = screenCoord + perturbation.xz * elevationRefFactor * elevationRefractionFactor;*/

	float waterRefrIndex = 1.33; //1.333;
	float targetDistUW = targetDist - eyeDist; // distance through water to target 0
	//float targetElevation = targetDistUW * sqrt(1.0 - pow(dot(eyeDir, smoothNormal), 2));
	
	vec3 T = refract(-eyeDir, normal, 1.0 / waterRefrIndex);
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

// mix reflection and refraction:
	float fresnelFactor = fresnel(normal, -eyeDir);
	vec4 final = vec4(mix(transmitColor.xyz, reflectColor.xyz, fresnelFactor), 1.0);

// fade out far edges of water
	//float targetElevationNormalized = targetElevation / 2.5;
	//float elevationAlphaFactor = min(1.0, pow(abs(targetElevationNormalized) * 10, 3));
	float alpha = /*elevationAlphaFactor */ (1-pow(fFog, 3.0));
	//final.a = alpha;

// DEBUG:
	//float f = pow(abs((T_targetElevation - transmitElevation) / T_targetElevation), 2.2);
	//float f = pow(transmitAttenuation, 2.2);
	//final = vec4(f, f, f, 1.0) + 0.00001 * final;
	//final = vec4(transmitColor.xyz, 1.0) + 0.00001 * final;
	//final.a = 0.00001;

	gl_FragColor = final;
}
