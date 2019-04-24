#pragma once

#include <boglfw/renderOpenGL/UniformPack.h>
#include <boglfw/utils/assert.h>

#include <glm/mat4x4.hpp>

class UPackTerrain : public UniformPack {
public:
	UPackTerrain();

	void setMatWorld(glm::mat4x4 val)				{ setUniform(iMatW_, val); }
	void setWaterNormalTextureSampler(int val)		{ setUniform(iTexWaterNormalSampler_, val); }
	void setTextureSampler(unsigned index, int val) {
		assertDbg(index < 5);
		setUniformIndexed(iTexSampler_, index, val);
	}

private:
	unsigned iMatW_;
	unsigned iTexSampler_;
	unsigned iTexWaterNormalSampler_;
};
