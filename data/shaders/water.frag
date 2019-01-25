#version 330 core

in vec3 fWPos;
in vec3 fNormal;
in vec4 fColor;
in vec2 fUV;

uniform sampler2D tex;

void main() {
	float lowFreqFactor = 0.05;
	// sample textures at two frequencies:
	vec4 t0 = texture(tex, fUV);
	vec4 t0low = texture(tex, fUV * lowFreqFactor);
	
	// mix texture frequencies
	t0 = (t0 * t0low) * 2;

	// compute lighting
	vec3 lightDir = normalize(vec3(1.0, -1.5, -0.3));
	float light = dot(-lightDir, normalize(fNormal));

	vec4 final = vec4(light * (fColor * t0).xyz, 1.0);

	// DEBUG:
	//final = vec4(t2.xyz, 1.0) + 0.01 * final;

	gl_FragColor = final;
}
