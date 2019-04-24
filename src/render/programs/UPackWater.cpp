#include "UPackWater.h"

UPackWater::UPackWater() {
	iTexFoam_ = addUniform({"textureFoam", UniformType::INT});
	iTexReflection_2D_ = addUniform({"textureReflection", UniformType::INT});
	iTexRefraction_ = addUniform({"textureRefraction", UniformType::INT});
	iTexRefraction_Cube_ = addUniform({"textureRefractionCube", UniformType::INT});
}


