#include "UPackTerrain.h"

UPackTerrain::UPackTerrain() {
	iTexSampler_ = addUniform({"tex", UniformType::INT, 5});
}
