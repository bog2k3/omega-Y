#include "UPackWaterSurface.h"

UPackWaterSurface::UPackWaterSurface() {
	iTexWaterNormalSampler_ = addUniform({"textureWaterNormal", UniformType::INT});
}
