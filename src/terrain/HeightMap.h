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

	// blurs the heightmap over a given radius
	void blur(float radius);

	// return the interpolated height value at position with normalized coordinates (u, v) <- [0.0, 1.0]
	float value(float u, float v) const;

private:
	unsigned width_;
	unsigned length_;
	float baseY_;
	float amplitude_;
	float* values_ = nullptr;

	struct element;

	void computeDiamondSquareStep(element* elements, unsigned r1, unsigned r2, unsigned c1, unsigned c2, float jitterAmp);
	void generate();
	float getSample(int r, int c) const;
	void normalizeValues();
};

#endif // HEIGHTMAP_H
