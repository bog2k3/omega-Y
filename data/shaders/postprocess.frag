#version 330 core

in vec2 fragUV;

uniform sampler2D texSampler;

out vec4 color;

void main() {
    vec3 val = texture2D(texSampler, fragUV).xyz;

	// do gamma correction
	float gamma = 2.2;
	vec3 gamma_inv = vec3(1.0 / gamma);
	val.xyz = pow(val.xyz, gamma_inv);

	color = vec4(val, 1.0);
}
