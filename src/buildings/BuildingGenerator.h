#ifndef __BUILDINGS_GENERATOR_H__
#define __BUILDINGS_GENERATOR_H__

struct BuildingsSettings {

};

class Terrain;

class BuildingGenerator {
public:
	static void generate(BuildingsSettings const& settings, Terrain &terrain);
};

#endif // __BUILDINGS_GENERATOR_H__
