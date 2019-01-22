#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

struct HeightmapParams {
	unsigned width;		// minimum width on X axis (actual may be greater)
	unsigned length;	// minimum length on Z axis (actual may be greater)
	float minHeight;	// lower displacement value on Y axis (may be negative)
	float maxHeight;	// higher displacement value on Y axis (may be negative)
	float bigRoughness = 1.f;	// roughness of big features (hills, mountains etc)
	float smallRoughness = 1.f;	// roughness of small features (small local variations in terrain)
};

class HeightMap
{
public:
	HeightMap(HeightmapParams const& params);
	virtual ~HeightMap();
	
	// return the interpolated height value at position (x, z)
	float value(float x, float z) const;
	
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
};

#endif // HEIGHTMAP_H
