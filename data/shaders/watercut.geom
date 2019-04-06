#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices=5) out;

in VertexData {
	vec3 fWPos;
	vec3 fNormal;
	vec4 fColor;
	vec2 fUV[5];
	vec4 fTexBlendFactor;
} vertexIn[];

out VertexData {
	vec3 fWPos;
	vec3 fNormal;
	vec4 fColor;
	vec2 fUV[5];
	vec4 fTexBlendFactor;
} vertexOut;

void emitMix(int i1, int i2) {
	float mixFactor = abs(gl_in[i1].gl_ClipDistance[0]) / (abs(gl_in[i1].gl_ClipDistance[0]) + abs(gl_in[i2].gl_ClipDistance[0]));
	gl_Position = mix(gl_in[i1].gl_Position, gl_in[i2].gl_Position, mixFactor);
	gl_ClipDistance[0] = 0.0;
	vertexOut.fWPos = mix(vertexIn[i1].fWPos, vertexIn[i2].fWPos, mixFactor);
	vertexOut.fNormal = mix(vertexIn[i1].fNormal, vertexIn[i2].fNormal, mixFactor);
	vertexOut.fColor = mix(vertexIn[i1].fColor, vertexIn[i2].fColor, mixFactor);
	for (int i=0; i<5; i++)
		vertexOut.fUV[i] = mix(vertexIn[i1].fUV[i], vertexIn[i2].fUV[i], mixFactor);
	vertexOut.fTexBlendFactor = mix(vertexIn[i1].fTexBlendFactor, vertexIn[i2].fTexBlendFactor, mixFactor);
	EmitVertex();
}

void emitSimple(int i) {
	gl_Position = gl_in[i].gl_Position;
	gl_ClipDistance[0] = gl_in[i].gl_ClipDistance[0];
	vertexOut.fWPos = vertexIn[i].fWPos;
	vertexOut.fNormal = vertexIn[i].fNormal;
	vertexOut.fColor = vertexIn[i].fColor;
	vertexOut.fUV = vertexIn[i].fUV;
	vertexOut.fTexBlendFactor = vertexIn[i].fTexBlendFactor;
	EmitVertex();
}

void main() {
	float clipDist[3] = float[3] (
		gl_in[0].gl_ClipDistance[0],
		gl_in[1].gl_ClipDistance[0],
		gl_in[2].gl_ClipDistance[0]
	);
	bool opp1 = clipDist[0] * clipDist[1] < 0; // is 1 opposite of 0?
	bool opp2 = clipDist[0] * clipDist[2] < 0; // is 2 opposite of 0?
	// choose as first the vertex the one that is alone on its side (or 0 if all are on the same side)
	int iFirst = (opp1 ^^ opp2)
		? (opp1 ? 1 : 2)
		: 0;

	// first vertex:
	emitSimple(iFirst);

	if (opp1 || opp2) {
		// create two new vertices where the edges intersect the water plane
		emitMix(iFirst, (iFirst+1)%3);
		emitMix(iFirst, (iFirst+2)%3);
	}

	// second vertex:
	emitSimple((iFirst+1)%3);

	// third vertex:
	emitSimple((iFirst+2)%3);

	EndPrimitive();
}
