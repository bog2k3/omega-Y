#pragma once

#include "SharedUniformPacks.h"
#include "UPackWaterSurface.h"

#include <boglfw/renderOpenGL/UniformPack.h>

#include <glm/mat4x4.hpp>

class UPackTerrain : public UniformPack {
public:
	UPackTerrain();

	void setMatWorld(glm::mat4x4 val)				{ setUniform(iMatW_, val); }
	void setTextureSampler(unsigned index, int val) { setUniformIndexed(iTexSampler_, index, val); }
	void setWaterNormalTexSampler(int val)			{ SharedUniformPacks::upWaterSurface->setWaterNormalTextureSampler(val); }

private:
	unsigned iMatW_;
	unsigned iTexSampler_;
};
