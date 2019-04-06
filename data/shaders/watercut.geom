#version 330 core

#include common.glsl
#include underwater.glsl

layout(triangles) in;
layout(triangle_strip, max_vertices=5) out;

in VertexData {
	vec3 normal;
	vec4 color;
	vec2 uv[5];
	vec4 texBlendFactor;
} vertexIn[];

out FragData {
	vec3 wPos;
	vec3 normal;
	vec4 color;
	vec2 uv[5];
	vec4 texBlendFactor;
} vertexOut;

uniform mat4 mPV;

vec4 project(vec3 wPos) {
	return mPV * vec4(wPos, 1);
}

vec3 applyRefraction(vec3 wPos) {
	//return wPos;
	if (bRefraction > 0 && wPos.y < 0)
		// refract the position of the vertex
		return refractPos(wPos, eyePos);
	else
		return wPos;
}

void emitMix(int i1, int i2) {
	float mixFactor = abs(gl_in[i1].gl_ClipDistance[0]) / (abs(gl_in[i1].gl_ClipDistance[0]) + abs(gl_in[i2].gl_ClipDistance[0]));

	vec3 wPos = mix(gl_in[i1].gl_Position.xyz, gl_in[i2].gl_Position.xyz, mixFactor);
	gl_Position = project(wPos);
	gl_ClipDistance[0] = 0.0;
	vertexOut.wPos = wPos;
	vertexOut.normal = mix(vertexIn[i1].normal, vertexIn[i2].normal, mixFactor);
	vertexOut.color = mix(vertexIn[i1].color, vertexIn[i2].color, mixFactor);
	for (int i=0; i<5; i++)
		vertexOut.uv[i] = mix(vertexIn[i1].uv[i], vertexIn[i2].uv[i], mixFactor);
	vertexOut.texBlendFactor = mix(vertexIn[i1].texBlendFactor, vertexIn[i2].texBlendFactor, mixFactor);
	EmitVertex();
}

void emitSimple(int i) {
	vec3 wPos = gl_in[i].gl_Position.xyz;
	vec3 pos = applyRefraction(wPos);
	gl_Position = project(pos);
	gl_ClipDistance[0] = gl_in[i].gl_ClipDistance[0];
	vertexOut.wPos = wPos;
	vertexOut.normal = vertexIn[i].normal;
	vertexOut.color = vertexIn[i].color;
	vertexOut.uv = vertexIn[i].uv;
	vertexOut.texBlendFactor = vertexIn[i].texBlendFactor;
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
