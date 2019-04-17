#ifndef TERRAIN_CONFIG_H
#define TERRAIN_CONFIG_H

struct TerrainConfig {
	// size settings
	float width = 100.f;	// X axis
	float length = 100.f;	// Z axis
	float minElevation = -5.f;	// minimum Y axis value
	float maxElevation = 15.f;	// maximum Y axis value

	// structure settings
	float vertexDensity = 2.f;	// vertices per meter (actual density may be slightly higher due to rounding,
								// but is guaranteed to always be at least the specified value)

	// generation parameters
	float relativeRandomJitter = 0.5f;	// random jitter applied to the vertex mesh in the XoZ plane;
										// a value of 1.0 means the amplitude of the jitter is equal
										// to the initial distance between vertices;
										// this has the effect of producing an irregular (less matrix-like) mesh

	float bigRoughness = 0.5f;		// between [0.0 and 1.0] -> controls roughness of big features (hills/cliffs etc)
	float smallRoughness = 1.f;		// roughness of small features (small local variations in terrain)
};

#endif // TERRAIN_CONFIG_H
