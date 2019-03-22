#version 330 core

in vec3 pos;
in vec3 normal;
in vec3 color;
in vec2 uv[5];
in vec4 texBlendFactor;

out vec3 fWPos;
out vec3 fNormal;
out vec4 fColor;
out vec2 fUV[5];
out vec4 fTexBlendFactor;

uniform mat4 mPV;
uniform float subspace;	// represents the subspace we're rendering: +1 above water, -1 below water
uniform int bRefraction;
uniform vec3 eyePos;

#define PI 3.1415

float approxRefractFn(float i0, float hr) {
	float r = 1.0 - 2.7 * i0 / PI;
	float p = 1.3 / (r+1) + pow(0.04 / hr, 2.0);
	r = pow(r, p);
	return clamp(r, 0, 1);
}

vec3 approxW(vec3 V, vec3 P, float i0, vec3 Wlim, vec3 P0, float n1, float n2, vec3 N) {
	float vh = V.y;
	float ph = -P.y;
	float hr = vh / (1 + ph);
	float r = approxRefractFn(i0, hr);
	return Wlim + (P0 - Wlim) * r;
}

const vec3 N_water = vec3(0, 1, 0);
const float n_air = 1.0;
const float n_water = 1.33;
const float t_lim = asin(n_air / n_water);
const float tan_tLim = tan(t_lim);

vec3 refractPos(vec3 wPos) {
	vec3 wPos0 = vec3(wPos.x, 0, wPos.z);
	vec3 V0 = vec3(eyePos.x, 0, eyePos.z);
	float i0 = acos(dot(normalize(eyePos - wPos), N_water));
	float tan_lim = tan_tLim;//i0 > t_lim ? tan_tLim : tan(i0);
	vec3 W_lim = wPos0 + normalize(V0 - wPos0) * tan_lim * abs(wPos.y);
	vec3 water_intersect = approxW(eyePos, wPos, i0, W_lim, wPos0, n_air, n_water, N_water);
	float uw_dist = length(wPos - water_intersect);
	vec3 newDir = normalize(water_intersect - eyePos);
	vec3 refracted = water_intersect + newDir * uw_dist;
	float fade_dist = 0.5;
	float depthFactor = clamp(-wPos.y / fade_dist, 0, 1); // fade refraction toward zero at water edges to avoid gaps
	vec3 final = mix(wPos, refracted, depthFactor);
	return final;
}

void main() {
	vec3 wPos = pos;
	if (bRefraction > 0) {
		// refract the position of the vertex
		wPos = refractPos(wPos);
	}
	gl_Position = mPV * vec4(wPos, 1);
	gl_ClipDistance[0] = pos.y * sign(subspace) + 0.2; //* (sign(subspace) == -1 ? 1 : 0);
	fWPos.xyz = pos; // ?wPos?
	//fWPos.w = gl_Position.z;
	fNormal = normal;
	fColor = vec4(color, 1);
	fUV[0] = uv[0];
	fUV[1] = uv[1];
	fUV[2] = uv[2];
	fUV[3] = uv[3];
	fUV[4] = uv[4];
	fTexBlendFactor = texBlendFactor;
}
