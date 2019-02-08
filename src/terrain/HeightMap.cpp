#include "HeightMap.h"

#include <boglfw/utils/rand.h>
#include <boglfw/math/math3D.h>
#include <boglfw/utils/log.h>

#include <cassert>

static const float jitterReductionFactor = 0.5f;	// jitter is multiplied by this factor at each iteration

// return the first power-of-two number greater than or equal to x
unsigned nextPo2(unsigned x) {
	assert((x << 1) > x);	// if x already uses the highest bit then we can't compute
	unsigned r = 1;
	while (r < x)
		r = r << 1;
	return r;
}

HeightMap::HeightMap(HeightmapParams const& params) {
	width_ = nextPo2(params.width) + 1;
	length_ = nextPo2(params.length) + 1;
	baseY_ = params.minHeight;
	elements_ = new element[width_ * length_];
	
	generate(params.maxHeight - params.minHeight);
}

HeightMap::~HeightMap()
{
	if (elements_)
		delete [] elements_, elements_ = nullptr;
}

float HeightMap::getSample(int r, int c) const {
	r = clamp(r, 0, (int)length_ - 1);
	c = clamp(c, 0, (int)width_ - 1);
	return elements_[r*width_ + c].value;
}

float HeightMap::value(float u, float v) const {
	u = clamp(u, 0.f, 1.f);
	v = clamp(v, 0.f, 1.f);
	// bilinear filtering by sampling the 4 nearest neighbours
	float rf = v * length_;		// floating point row coord
	float cf = u * width_;		// floating point column coord
	unsigned r = (unsigned) rf;	// row index
	unsigned c = (unsigned) cf;	// column index
	float uWeight = cf - c - 0.5f;	// u blending weight
	float vWeight = rf - r - 0.5f;	// v blending weight
	if (uWeight < 0) {
		c--;
		uWeight += 1.f;
	}
	if (vWeight < 0) {
		r--;
		vWeight += 1.f;
	}
	float f1 = getSample(r, c) * (1.f - uWeight) + getSample(r, c+1) * uWeight;
	float f2 = getSample(r+1, c) * (1.f - uWeight) + getSample(r+1, c+1) * uWeight;
	return baseY_ + f1 * (1.f - vWeight) + f2 * vWeight;
}

void HeightMap::computeDiamondSquareStep(unsigned r1, unsigned r2, unsigned c1, unsigned c2, float jitterAmp) {
	unsigned midR = (r1 + r2) / 2;
	unsigned midC = (c1 + c2) / 2;
	// center (diamond step)
	elements_[midR * width_ + midC] += 0.25f * (
		elements_[r1 * width_ + c1].get() +
		elements_[r1 * width_ + c2].get() +
		elements_[r2 * width_ + c1].get() +
		elements_[r2 * width_ + c2].get()
	) + srandf() * jitterAmp;
	float centerValue = elements_[midR * width_ + midC].get();
	// now square step:
	// top midpoint:
	if (c2 > c1+1)
		elements_[r1 * width_ + midC] += 0.33f * (
			elements_[r1 * width_ + c1].get() +
			elements_[r1 * width_ + c2].get() +
			centerValue
		) + srandf() * jitterAmp;
	// bottom midpoint:
	if (c2 > c1+1)
		elements_[r2 * width_ + midC] += 0.33f * (
			elements_[r2 * width_ + c1].get() +
			elements_[r2 * width_ + c2].get() +
			centerValue
		) + srandf() * jitterAmp;
	// left midpoint:
	if (r2 > r1+1)
		elements_[midR * width_ + c1] += 0.33f * (
			elements_[r1 * width_ + c1].get() +
			elements_[r2 * width_ + c1].get() +
			centerValue
		) + srandf() * jitterAmp;
	// right midpoint:
	if (r2 > r1+1)
		elements_[midR * width_ + c2] += 0.33f * (
			elements_[r1 * width_ + c2].get() +
			elements_[r2 * width_ + c2].get() +
			centerValue
		) + srandf() * jitterAmp;
	if (c2 > c1+2 || r2 > r1+2) {
		// recurse into the 4 sub-sections:
		// top-left
		computeDiamondSquareStep(r1, midR, c1, midC, jitterAmp * jitterReductionFactor);
		// top-right
		computeDiamondSquareStep(r1, midR, midC, c2, jitterAmp * jitterReductionFactor);
		// bottom-left
		computeDiamondSquareStep(midR, r2, c1, midC, jitterAmp * jitterReductionFactor);
		// bottom-right
		computeDiamondSquareStep(midR, r2, midC, c2, jitterAmp * jitterReductionFactor);
	}
}

void HeightMap::generate(float amplitude) {
	// set the corners:
	elements_[0] += amplitude* (0.5f + 0.5f*randf());
	elements_[width_-1] += amplitude* (0.5f + 0.5f*randf());
	elements_[(length_-1)*width_] += amplitude* (0.5f + 0.5f*randf());
	elements_[width_ * length_ - 1] += amplitude* (0.5f + 0.5f*randf());
	float jitterAmp = amplitude * jitterReductionFactor;
	// compute midpoint displacement recursively:
	computeDiamondSquareStep(0, length_-1, 0, width_-1, jitterAmp);
	// average out the values and compute min/max:
	float vmin = 1e20f, vmax = -1e20f;
	for (unsigned i=0; i<width_*length_; i++) {
		elements_[i].value /= elements_[i].divider, elements_[i].divider = 1;
		if (elements_[i].value < vmin)
			vmin = elements_[i].value;
		if (elements_[i].value > vmax)
			vmax = elements_[i].value;
	}
	// renormalize the values to fill the entire height range
	float scale = amplitude / (vmax - vmin);
	for (unsigned i=0; i<width_*length_; i++)
		elements_[i].value = (elements_[i].value - vmin) * scale;
}

void HeightMap::meltEdges(unsigned radius) {
	if (radius > width_/2 || radius > length_/2) {
		ERROR("HeightMap::meltEdges() received invalid parameter");
		return;
	}
	// top edge:
	for (unsigned i=0; i<radius; i++) {
		float f = pow((float)i / radius, 2.f);
		for (unsigned j=0; j<width_; j++)
			elements_[i*width_+j].value = lerp(0.f, elements_[i*width_+j].value, f);
	}
	// bottom edge:
	for (unsigned i=length_-radius-1; i<length_; i++) {
		float f = pow((float)(length_-1-i) / radius, 2.f);
		for (unsigned j=0; j<width_; j++)
			elements_[i*width_+j].value = lerp(0.f, elements_[i*width_+j].value, f);
	}
	// left edge:
	for (unsigned j=0; j<radius; j++) {
		float f = pow((float)j / radius, 2.f);
		for (unsigned i=0; i<length_; i++)
			elements_[i*width_+j].value = lerp(0.f, elements_[i*width_+j].value, f);
	}
	// right edge:
	for (unsigned j=width_-radius-1; j<width_; j++) {
		float f = pow((float)(width_-1-j) / radius, 2.f);
		for (unsigned i=0; i<length_; i++)
			elements_[i*width_+j].value = lerp(0.f, elements_[i*width_+j].value, f);
	}
}
