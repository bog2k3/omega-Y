#ifndef TERRAIN_CONFIG_H
#define TERRAIN_CONFIG_H

#include <cstdint>

struct TerrainConfig {
	// random seed
	float seed = 0;

	// size settings
	float width = 200.f;	// X axis
	float length = 200.f;	// Z axis
	float minElevation = -10.f;	// minimum Y axis value
	float maxElevation = 25.f;	// maximum Y axis value

	// structure settings
	float vertexDensity = 1.f;	// vertices per meter (actual density may be slightly higher due to rounding,
								// but is guaranteed to always be at least the specified value)

	// generation parameters
	float relativeRandomJitter = 0.8f;	// random jitter applied to the vertex mesh in the XoZ plane;
										// a value of 1.0 means the amplitude of the jitter is equal
										// to the initial distance between vertices;
										// this has the effect of producing an irregular (less matrix-like) mesh

	float roughness = 0.5f;		// between [0.0 and 1.0]
};

#endif // TERRAIN_CONFIG_H
