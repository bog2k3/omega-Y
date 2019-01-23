#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#include <glm/vec2.hpp>

/*
 * tutorial: https://gpfault.net/posts/perlin-noise.txt.html
 */

class PerlinNoise
{
public:
	// construct a new 2D perlin noise function with a lattice of [width]x[height] size.
	PerlinNoise(unsigned width, unsigned height);
	
	// construct a new 3D perlin noise function with a lattice of [width]x[height]x[length] size.
	PerlinNoise(unsigned width, unsigned height, unsigned length);
	
	virtual ~PerlinNoise();

	// returns the perlin noise function at (u,v).
	// coordinates 0.0 and 1.0 correspond to the edges of the noise map and 
	// coordinates outside of this range are wrapped around automatically
	float get(float u, float v);
	
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
