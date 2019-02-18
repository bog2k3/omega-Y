#version 330 core

in vec3 fWPos;
in vec3 fNormal;
in vec4 fColor;
in vec2 fUV;

uniform sampler2D textureDuDv;
uniform samplerCube textureReflection;

void main() {
	vec4 dudv = texture(textureDuDv, fUV) * 2.0 - 1.0;
	dudv += texture(textureDuDv, fUV * 5) * 2.0 - 1.0;

	vec3 normal = vec3(0.0, 1.0, 0.0);
	float perturbStrength = 0.5;
	normal = normalize(normal + vec3(dudv.r, 0.0, dudv.g) * perturbStrength);

	// compute lighting
	vec3 lightDir = normalize(vec3(1.0, -1.5, -0.3));
	float light = dot(-lightDir, normal);

	vec4 final = vec4(vec3(0.8, 0.9, 1.0) * vec3(light), 0.7);

	// DEBUG:
	//final = vec4(fWPos + fNormal, 1.0) + fColor;
	//final.xy += fUV;
	//final = vec4(normal, 1.0) + 0.00001 * final;

	gl_FragColor = final;
}
