#include "PerlinNoise.h"

#include <boglfw/utils/rand.h>

#include <glm/geometric.hpp>

#include <algorithm>
#include <vector>

PerlinNoise::PerlinNoise(unsigned width, unsigned height)
	: width_(width)
	, height_(height)
{
	pGradients_ = (glm::vec2*)malloc(sizeof(glm::vec2)*(width+1) * (height+1));
	generate();
}

PerlinNoise::~PerlinNoise()
{
	if (pGradients_)
		free(pGradients_), pGradients_ = nullptr;
}

// 5th degree polynomial function for S-curve interpolation
float srpPolynomial(float t) {
	return t*t*t*(t*(t*6.0 - 15.0) + 10.0);
}

int wrap(int x, int size) {
	while (x < 0)
		x += size;
	while (x >= size)
		x -= size;
	return x;
}

// changes the contrast of a distribution by adjusting the separation between low and high values
float contrastFn(float x, float c) {
	if (x > 0)
		return pow(x, 1.f / c);
	else if (x == 0)
		return x;
	else
		return -pow(-x, 1.f / c);
}

float PerlinNoise::get(float u, float v, float contrast) {
	glm::vec2 pf {u * (width_+1), v * (height_+1)};
	
	unsigned c0 = floor(pf.x);	// left column
	unsigned r0 = floor(pf.y); 	// top row
	glm::vec2 p0(c0, r0);		// top left lattice point
	glm::vec2 p1(c0+1, r0);		// top right lattice point
	glm::vec2 p2(c0, r0+1);		// bottom left lattice point
	glm::vec2 p3(c0+1, r0+1);	// bottom right lattice point
	
	float uF = srpPolynomial(pf.x - c0);	// u interpolation factor
	float vF = srpPolynomial(pf.y - r0);	// v interpolation factor
	
	// sample gradient vectors from lattice:
	glm::vec2 g0 = pGradients_[wrap(p0.y, height_+1) * (width_+1) + wrap(p0.x, width_+1)];
	glm::vec2 g1 = pGradients_[wrap(p1.y, height_+1) * (width_+1) + wrap(p1.x, width_+1)];
	glm::vec2 g2 = pGradients_[wrap(p2.y, height_+1) * (width_+1) + wrap(p2.x, width_+1)];
	glm::vec2 g3 = pGradients_[wrap(p3.y, height_+1) * (width_+1) + wrap(p3.x, width_+1)];
	
	float samp01 = (1.f - uF) * glm::dot(g0, pf - p0) + uF * glm::dot(g1, pf - p1);
	float samp23 = (1.f - uF) * glm::dot(g2, pf - p2) + uF * glm::dot(g3, pf - p3);
	
	float value = (1.f - vF) * samp01 + vF * samp23;
	return contrastFn(value, contrast);	// adjust contrast
}

float PerlinNoise::getNorm(float u, float v, float contrast) {
	return get(u, v, contrast) * 0.5f + 0.5f;
}

#ifdef DEBUG
glm::vec2 PerlinNoise::getGradientVector(float u, float v) {
	glm::vec2 pf {u * width_, v * height_};
	unsigned c0 = floor(pf.x);	// left column
	unsigned r0 = floor(pf.y); 	// top row
	glm::vec2 p0(c0, r0);		// top left lattice point
	glm::vec2 p1(c0+1, r0);		// top right lattice point
	glm::vec2 p2(c0, r0+1);		// bottom left lattice point
	glm::vec2 p3(c0+1, r0+1);	// bottom right lattice point
	
	float uF = pf.x - c0;	// u interpolation factor
	float vF = pf.y - r0;	// v interpolation factor
	
	// sample gradient vectors from lattice:
	glm::vec2 g0 = pGradients_[wrap(p0.y, height_+1) * (width_+1) + wrap(p0.x, width_+1)];
	glm::vec2 g1 = pGradients_[wrap(p1.y, height_+1) * (width_+1) + wrap(p1.x, width_+1)];
	glm::vec2 g2 = pGradients_[wrap(p2.y, height_+1) * (width_+1) + wrap(p2.x, width_+1)];
	glm::vec2 g3 = pGradients_[wrap(p3.y, height_+1) * (width_+1) + wrap(p3.x, width_+1)];
	
	glm::vec2 s01 = glm::normalize((1.f - uF) * g0 + uF * g1);
	glm::vec2 s23 = glm::normalize((1.f - uF) * g2 + uF * g3);
	
	return glm::normalize((1.f - vF) * s01 + vF * s23);
}
#endif

void PerlinNoise::generate() {
	// generate gradient template vectors
	const unsigned nTemplates = 128;
	glm::vec2 gradTemplates[nTemplates];
	for (unsigned i=0; i<nTemplates; i++) {
		gradTemplates[i] = glm::normalize(glm::vec2(srandf(), srandf()));
	}
	// generate gradient template indexes
	std::vector<unsigned> tIndex(nTemplates, 0);
	for (unsigned i=0; i<nTemplates; i++)
		tIndex[i] = i;
	std::random_shuffle(tIndex.begin(), tIndex.end());
	// compute lattice gradient vectors
	for (unsigned i=0; i<=height_; i++) {
		for (unsigned j=0; j<=width_; j++) {
			unsigned index = tIndex[(tIndex[i % nTemplates] + j) % nTemplates];
			pGradients_[i*(width_+1) + j] = gradTemplates[index];
		}
	}
}
