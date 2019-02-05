#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#include <glm/vec2.hpp>

/*
 * tutorial: https://gpfault.net/posts/perlin-noise.txt.html
 */

class PerlinNoise {
public:
	// construct a new 2D perlin noise function with a lattice of [width]x[height] size.
	PerlinNoise(unsigned width, unsigned height);

	// construct a new 3D perlin noise function with a lattice of [width]x[height]x[length] size.
	PerlinNoise(unsigned width, unsigned height, unsigned length);

	virtual ~PerlinNoise();

	// returns the perlin noise function at (u,v).
	// coordinates 0.0 and 1.0 correspond to the edges of the noise map and
	// coordinates outside of this range are wrapped around automatically.
	// return values are mapped to [-1.0, +1.0] interval.
	// [contrast] controls the steepness of transition between low values and high values.
	// [contrast] of 1.0 means smooth, linear transition, while > 1.0 means a higher contrast, with more separation between highs and lows
	float get(float u, float v, float contrast);

	// same as get, except the return value is normalized into [0.0, +1.0]
	float getNorm(float u, float v, float contrast);

#ifdef DEBUG
	glm::vec2 getGradientVector(float u, float v);
#endif

private:
	glm::vec2* pGradients_  = nullptr;
	unsigned width_ = 0;
	unsigned height_ = 0;

	void generate();
};

#endif // PERLINNOISE_H
