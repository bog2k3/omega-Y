#ifndef __VEGETATION_GENERATOR_H__
#define __VEGETATION_GENERATOR_H__

struct VegetationSettings {

};

class Terrain;

class VegetationGenerator {
public:
	static void generate(VegetationSettings const& settings, Terrain &terrain);
};

#endif // __VEGETATION_GENERATOR_H__
