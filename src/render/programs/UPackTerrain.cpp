#include "UPackTerrain.h"

UPackTerrain::UPackTerrain() {
	iMatW_ = addUniform({"matW", UniformType::MAT4});
	iTexSampler_ = addUniform({"tex", UniformType::INT, 5});
}
