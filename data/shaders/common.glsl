#ifndef COMMON_GLSL
#define COMMON_GLSL

#define PI 3.1415

const float Zn = 0.15;
const float Zf = 500.0;
const float fov = PI/2.5; // vertical field of view

uniform float subspace;	// represents the subspace we're rendering: +1 above water, -1 below water
uniform vec3 eyePos;
uniform float time;

#endif // COMMON_GLSL
