#include "HeightMap.h"
#include "../imgUtil/blur.h"

#include <boglfw/utils/rand.h>
#include <boglfw/math/math3D.h>
#include <boglfw/utils/log.h>

#include <cassert>
#include <vector>

static const float jitterReductionFactor = 0.5f;	// jitter is multiplied by this factor at each iteration

struct HeightMap::element {
	float value = 0.f;
	unsigned divider = 0;

	element& operator += (float val) {
		value += val;
		divider++;
		return *this;
	}
	float get() const { return value / divider; }
};

// return the first power-of-two number greater than or equal to x
unsigned nextPo2(unsigned x) {
	assertDbg((x << 1) > x);	// if x already uses the highest bit then we can't compute
	unsigned r = 1;
	while (r < x)
		r = r << 1;
	return r;
}

HeightMap::HeightMap(HeightmapParams const& params) {
	width_ = nextPo2(params.width) + 1;
	length_ = nextPo2(params.length) + 1;
	baseY_ = params.minHeight;
	amplitude_ = params.maxHeight - params.minHeight;
	values_ = new float[width_ * length_];

	generate();
}

HeightMap::~HeightMap()
{
	if (values_)
		delete [] values_, values_ = nullptr;
}

float HeightMap::getSample(int r, int c) const {
	r = clamp(r, 0, (int)length_ - 1);
	c = clamp(c, 0, (int)width_ - 1);
	return values_[r*width_ + c];
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

void HeightMap::computeDiamondSquareStep(element* elements, unsigned r1, unsigned r2, unsigned c1, unsigned c2, float jitterAmp) {
	unsigned midR = (r1 + r2) / 2;
	unsigned midC = (c1 + c2) / 2;
	// center (diamond step)
	elements[midR * width_ + midC] += 0.25f * (
		elements[r1 * width_ + c1].get() +
		elements[r1 * width_ + c2].get() +
		elements[r2 * width_ + c1].get() +
		elements[r2 * width_ + c2].get()
	) + srandf() * jitterAmp;
	float centerValue = elements[midR * width_ + midC].get();
	// now square step:
	// top midpoint:
	if (c2 > c1+1)
		elements[r1 * width_ + midC] += 0.3333f * (
			elements[r1 * width_ + c1].get() +
			elements[r1 * width_ + c2].get() +
			centerValue
		) + srandf() * jitterAmp;
	// bottom midpoint:
	if (c2 > c1+1)
		elements[r2 * width_ + midC] += 0.3333f * (
			elements[r2 * width_ + c1].get() +
			elements[r2 * width_ + c2].get() +
			centerValue
		) + srandf() * jitterAmp;
	// left midpoint:
	if (r2 > r1+1)
		elements[midR * width_ + c1] += 0.3333f * (
			elements[r1 * width_ + c1].get() +
			elements[r2 * width_ + c1].get() +
			centerValue
		) + srandf() * jitterAmp;
	// right midpoint:
	if (r2 > r1+1)
		elements[midR * width_ + c2] += 0.3333f * (
			elements[r1 * width_ + c2].get() +
			elements[r2 * width_ + c2].get() +
			centerValue
		) + srandf() * jitterAmp;
}

void HeightMap::generate() {
	element* elements = new element[width_ * length_];
	// seed the corners:
	elements[0] += amplitude_ * randf();
	elements[width_-1] += amplitude_ * randf();
	elements[(length_-1)*width_] += amplitude_ * randf();
	elements[width_ * length_ - 1] += amplitude_ * randf();
	// seed the center...
	elements[(length_-1)/2*width_ + (width_-1)/2] += amplitude_ * randf();
	// ...and the 4 edge mid-points as well
	elements[(width_-1)/2] += amplitude_ * randf();
	elements[(length_-1)*width_ + (width_-1)/2] += amplitude_ * randf();
	elements[(length_-1)/2*width_] += amplitude_ * randf();
	elements[(length_-1)/2*width_ + (width_-1)] += amplitude_ * randf();

	struct rcData {
		unsigned r1, r2, c1, c2;
		float jitterAmp;
	};
	std::vector<rcData> vSteps;
	vSteps.reserve(width_*width_ / 3.5f);

	vSteps.push_back(rcData{
		0, length_ - 1,
		0, width_ - 1,
		amplitude_ * jitterReductionFactor
	});
	unsigned index = 0;
	// compute diamond displacement iteratively:
	while (index < vSteps.size()) {
		unsigned r1 = vSteps[index].r1;
		unsigned r2 = vSteps[index].r2;
		unsigned c1 = vSteps[index].c1;
		unsigned c2 = vSteps[index].c2;
		computeDiamondSquareStep(elements, r1, r2, c1, c2, vSteps[index].jitterAmp);
		if (c2 > c1+2 || r2 > r1+2) {
			unsigned midR = (r1 + r2) / 2;
			unsigned midC = (c1 + c2) / 2;
			// top-left
			vSteps.push_back(rcData{r1, midR, c1, midC, vSteps[index].jitterAmp * jitterReductionFactor});
			// top-right
			vSteps.push_back(rcData{r1, midR, midC, c2, vSteps[index].jitterAmp * jitterReductionFactor});
			// bottom-left
			vSteps.push_back(rcData{midR, r2, c1, midC, vSteps[index].jitterAmp * jitterReductionFactor});
			// bottom-right
			vSteps.push_back(rcData{midR, r2, midC, c2, vSteps[index].jitterAmp * jitterReductionFactor});
		}
		index++;
	}
	// bake the values:
	for (unsigned i=0; i<width_*length_; i++) {
		values_[i] = elements[i].value / elements[i].divider;
	}
	delete [] elements, elements = nullptr;

	// renormalize the values to fill the entire height range
	normalizeValues();
}

void HeightMap::normalizeValues() {
	float vmin = 1e20f, vmax = -1e20f;
	for (unsigned i=0; i<width_*length_; i++) {
		if (values_[i] < vmin)
			vmin = values_[i];
		if (values_[i] > vmax)
			vmax = values_[i];
	}
	float scale = amplitude_ / (vmax - vmin);
	for (unsigned i=0; i<width_*length_; i++) {
		values_[i] = (values_[i] - vmin) * scale;
	}
}

void HeightMap::blur(float radius) {
	float *newValues = new float[width_*length_];
	imgUtil::blur(values_, length_, width_, radius, newValues);
	delete [] values_;
	values_ = newValues;
	normalizeValues();
}
