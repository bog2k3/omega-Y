#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

struct HeightmapParams {
	unsigned width;		// minimum width on X axis (actual may be greater)
	unsigned length;	// minimum length on Z axis (actual may be greater)
	float minHeight;	// lower displacement value on Y axis (may be negative)
	float maxHeight;	// higher displacement value on Y axis (may be negative)
};

class HeightMap {
public:
	HeightMap(HeightmapParams const& params);
	virtual ~HeightMap();

	// melt the edges over a [radius] distance from the edge, bringing their height value down gradually to zero
	void meltEdges(unsigned radius);

	// return the interpolated height value at position with normalized coordinates (u, v) <- [0.0, 1.0]
	float value(float u, float v) const;

private:
	unsigned width_;
	unsigned length_;
	float baseY_;
	struct element {
		float value = 0.f;
		unsigned divider = 0;

		element& operator += (float val) {
			value += val;
			divider++;
			return *this;
		}
		float get() const { return value / divider; }
	};
	element *elements_ = nullptr;

	void computeMidpointStep(unsigned r1, unsigned r2, unsigned c1, unsigned c2, float jitterAmp);
	void generate(float amplitude);
	float getSample(int r, int c) const;
};

#endif // HEIGHTMAP_H
