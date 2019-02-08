#ifndef __BUILDINGS_GENERATOR_H__
#define __BUILDINGS_GENERATOR_H__

struct BuildingsSettings {
	int numCastles = 1;					// you should have around 1-2 castles per 200x200 meter^2
	float smallBuildingDensity = 1.5f;	// number of small buildings per 100x100 meter^2
};

class Terrain;

class BuildingGenerator {
public:
	static void generate(BuildingsSettings const& settings, Terrain &terrain);
};

#endif // __BUILDINGS_GENERATOR_H__
