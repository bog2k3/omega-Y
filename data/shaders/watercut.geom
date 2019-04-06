#version 330 core

layout(triangles) in;
layout(triangle_strip, max_vertices=3) out;

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

void main() {
	for (int i=0; i<3; i++) {
		gl_Position = gl_in[i].gl_Position;
		gl_ClipDistance[0] = gl_in[i].gl_ClipDistance[0];
		vertexOut.fWPos = vertexIn[i].fWPos;
		vertexOut.fNormal = vertexIn[i].fNormal;
		vertexOut.fColor = vertexIn[i].fColor;
		vertexOut.fUV = vertexIn[i].fUV;
		vertexOut.fTexBlendFactor = vertexIn[i].fTexBlendFactor;
		EmitVertex();
	}
	EndPrimitive();
}
