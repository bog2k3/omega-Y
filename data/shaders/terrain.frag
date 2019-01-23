in vec3 fWPos;
in vec3 fNormal;
in vec4 fColor;
in vec2 fUV[4];
in float fTexWeight[4];

uniform sampler2D tex[4];

void main() {
	vec4 dummy = vec4(fUV[0].x, fUV[1].x, fUV[2].x, fUV[3].x) + vec4(fNormal, 0) + vec4(fTexWeight[0], fTexWeight[1], fTexWeight[2], fTexWeight[3]);

	vec3 lightPoint = vec3(0.0, 30.0, 0.0);
	//vec3 lightDir = normalize(vec3(1.0, -1.5, -0.3));
	vec3 lightVec = fWPos - lightPoint;
	float lightDist = length(lightVec);
	vec3 lightDir = lightVec / lightDist;
	float light = dot(-lightDir, normalize(fNormal));
	float falloff = 1000.0 / (lightDist*lightDist);

	dummy += light;

	gl_FragColor = light*falloff*fColor + dummy*0.01;// * texture(tex1, fUV1);
	//vec3 nc = normalize(fNormal) * 0.5 + 0.5;
	//nc.x = pow(nc.x, 4.0);
	//nc.y = pow(nc.y, 4.0);
	//nc.z = pow(nc.z, 4.0);
	//nc *= 0.75;
	//gl_FragColor = vec4(nc, 1) + dummy*0.001;
}
