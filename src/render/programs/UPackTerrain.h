#pragma once

#include "SharedUniformPacks.h"
#include "UPackWaterSurface.h"

#include <boglfw/renderOpenGL/UniformPack.h>

#include <glm/mat4x4.hpp>

class UPackTerrain : public UniformPack {
public:
	UPackTerrain();

	void setTextureSampler(unsigned index, int val) { setUniformIndexed(iTexSampler_, index, val); }
	void setWaterNormalTexSampler(int val)			{ SharedUniformPacks::upWaterSurface->setWaterNormalTextureSampler(val); }

private:
	unsigned iTexSampler_;
};
