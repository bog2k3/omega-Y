#version 330 core

in vec3 fWPos;
in vec3 fNormal;
in vec2 fUV;
in float fFog;
in vec3 fScreenUV;

uniform float time;
uniform vec3 eyePos;
uniform sampler2D textureDuDv;
uniform samplerCube textureReflection;
uniform sampler2D textureRefraction;

#define PI 3.1415

float fresnel(float n1, float n2, vec3 normal, vec3 incident) {
	// Schlick aproximation
	/*float r0 = (n1-n2) / (n1+n2);
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
	return fres;*/

	float f = 1.0 - pow(dot(normal, -incident), 2.0);
	return min(1.0, pow(f, 10) * 1.3);
 }

void main() {
	vec3 normal = vec3(0.0, 1.0, 0.0);
	vec3 eyeDir = eyePos - fWPos;
	float eyeDist = length(eyeDir);
	float eyeDistProjected = length(vec3(eyeDir.x, 0.0, eyeDir.z));
	float eyeHeight = eyeDir.y;
	eyeDir /= eyeDist; // normalize

	float changeSpeed = 0.02;
	float change = time * changeSpeed;

// compute perturbation of normal
	/*float perturbFreq1 = 0.5;
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

	float perturbFreq4 = 20.0;
	float perturbStrength4 = 0.02;
	dudv = texture(textureDuDv, (fUV.yx + change/5) * perturbFreq4) * 2.0 - 1.0;
	vec3 perturb4 = dudv.rbg * perturbStrength4;

	float perturbFreq5 = 60.0;
	float perturbStrength5 = 0.05;
	dudv = texture(textureDuDv, (fUV.yx - change/7) * perturbFreq5) * 2.0 - 1.0;
	vec3 perturb5 = dudv.rbg * perturbStrength5;

	float perturbTotalFactor = (1.0 - pow(fFog, 0.8)) * 0.5;
	vec3 perturbTotal = perturbTotalFactor * (perturb1 + perturb2 + perturb3 + perturb4 + perturb5);
	perturbTotal.y = 0;*/

	float wavefrontSpeed = 2.f; // m/s
	vec2 dh_p0 = texture(textureDuDv, fUV*3).rg;
	float dh = pow(dh_p0.x, 1.0) * PI/200; // delta heading [rad]
	float p0 = dh_p0.y * PI * 0.5; // initial phase [rad]
	float wavefrontDist = length(fWPos) / (1 + dh); // m

	//float baseLambda = length(dFdy(fWPos)) * 40; // minimum wavelength we can render at nyquist frequency
	float baseLambda = length(dFdy(fWPos)) * 40; // minimum wavelength we can render at nyquist frequency
	float freq1 = 0.2; // Hz
	float amp1 = 0.5; // m
	amp1 *= min(1.0, pow(freq1 * wavefrontSpeed / baseLambda, 0.8));
	float wave1 = amp1 * sin(p0 + 2*PI*freq1 * (time*0 + wavefrontDist / wavefrontSpeed));

	float freq2 = 0.45; // Hz
	float amp2 = 0.3; // m
	amp2 *= min(1.0, pow(freq2 * wavefrontSpeed / baseLambda, 1));
	float wave2 = amp2 * sin(p0 + 2*PI*freq2 * (time + wavefrontDist / wavefrontSpeed));

	float freq3 = 2.0; // Hz
	float amp3 = 0.25; // m
	amp3 *= min(1.0, pow(freq3 * wavefrontSpeed / baseLambda, 2));
	float wave3 = amp3 * sin(p0 + 2*PI*freq3 * (time + wavefrontDist / wavefrontSpeed));

	float waveTotal = wave1 + wave2 + wave3;

	float perturbFogFactor = 1.0 - pow(fFog, 0.8);
	float perturbTotalFactor = perturbFogFactor;
	vec3 perturbTotal = perturbTotalFactor * vec3(dFdx(waveTotal), 0, dFdy(waveTotal));

// alter normal
	normal = normalize(normal + perturbTotal);

// compute reflection
	vec3 reflectDir = reflect(-eyeDir, normal);
	vec4 reflectColor = texture(textureReflection, reflectDir);

	vec3 reflectTint = vec3(0.5, 0.6, 0.65) * 1.2;
	reflectColor.xyz *= reflectTint;

// towards the far edges, reflection fades to skybox
	vec3 fogDir = -eyeDir;
	fogDir.y = 0;
	vec4 fogColor = texture(textureReflection, fogDir);
	float fogFactor = pow(fFog, 0.40);
	reflectColor.xyz = mix(reflectColor.xyz, fogColor.xyz, fogFactor);

// compute refraction
	vec2 refractCoord = fScreenUV.xy / fScreenUV.z * 0.5 + 0.5;
	float targetElevation = texture(textureRefraction, refractCoord).a;
	float elevationRefFactor = abs(targetElevation - 0.5) * 2;
	refractCoord += perturbTotal.xz * elevationRefFactor / pow(eyeDist, 0.5);
	vec4 refractColor = texture(textureRefraction, refractCoord);

// mix reflection and refraction:
	float fresnelFactor = fresnel(1.0, 1.0, normal, -eyeDir);
	vec4 final = vec4(mix(refractColor.xyz, reflectColor.xyz, fresnelFactor), 1.0);

	float alpha = pow(elevationRefFactor, 0.5);
	alpha *= (1-pow(fFog, 3.0));
	final.a = alpha;

	// DEBUG:
	float f = (waveTotal + 1) * 0.5;
	//final = vec4(f, f, f, 1.0) + 0.00001 * final;
	//final = vec4(dFdx(perturbTotal) * 30 + 0.1, 1.0) + 0.00001 * final;
	//final.a = 0.00001;

	gl_FragColor = final;
}
