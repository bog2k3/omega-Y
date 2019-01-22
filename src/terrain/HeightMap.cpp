#include "HeightMap.h"

#include <boglfw/utils/rand.h>

#include <cassert>

static const float jitterReductionFactor = 0.25f;

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

float HeightMap::value(float x, float z) const {
}

void HeightMap::computeMidpointStep(unsigned r1, unsigned r2, unsigned c1, unsigned c2, float jitterAmp) {
	unsigned midR = (r1 + r2) / 2;
	unsigned midC = (c1 + c2) / 2;
	// top midpoint:
	if (c2 > c1+1)
		elements_[r1 * width_ + midC] += 0.5f * (elements_[r1 * width_ + c1].get() + elements_[r1 * width_ + c2].get()) + srandf() * jitterAmp;
	// bottom midpoint:
	if (c2 > c1+1)
		elements_[r2 * width_ + midC] += 0.5f * (elements_[r2 * width_ + c1].get() + elements_[r2 * width_ + c2].get()) + srandf() * jitterAmp;
	// left midpoint:
	if (r2 > r1+1)
		elements_[midR * width_ + c1] += 0.5f * (elements_[r1 * width_ + c1].get() + elements_[r2 * width_ + c1].get()) + srandf() * jitterAmp;
	// right midpoint:
	if (r2 > r1+1)
		elements_[midR * width_ + c2] += 0.5f * (elements_[r1 * width_ + c2].get() + elements_[r2 * width_ + c2].get()) + srandf() * jitterAmp;
	// center:
	if (c2 > c1+1 && r2 > r1+1)
		elements_[midR * width_ + midC] += 0.25f * (
				elements[r1 * width_ + midC].get() + 
				elements[r2 * width_ + midC].get() + 
				elements[midR * width_ + c1].get() + 
				elements[midR * width_ + c2].get()
			) + srandf() * jitterAmp;
	if (c2 > c1+2 || r2 > r1+2) {
		// recurse into the 4 sub-sections:
		// top-left
		computeMidpointStep(r1, midR, c1, midC, jitterAmp * jitterReductionFactor);
		// top-right
		computeMidpointStep(r1, midR, midC, c2, jitterAmp * jitterReductionFactor);
		// bottom-left
		computeMidpointStep(midR, r2, c1, midC, jitterAmp * jitterReductionFactor);
		// bottom-right
		computeMidpointStep(midR, r2, midC, c2, jitterAmp * jitterReductionFactor);
	}
}

void HeightMap::generate(float amplitude) {
	// set the corners:
	elements_[0] += randf() * amplitude;
	elements_[width_-1] += randf() * amplitude;
	elements_[(length_-1)*width_] += randf() * amplitude;
	elements_[width_ * length_ - 1] += randf() * amplitude;
	float jitterAmp = amplitude * jitterReductionFactor;
	// compute midpoint displacement recursively:
	computeMidpointStep(0, length_-1, 0, width_-1, jitterAmp);
	// average out the values:
	for (unsigned i=0; i<width_*length_; i++)
		elements_[i].value /= elements_[i].divider, elements_[i].divider = 1;
}