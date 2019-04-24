#pragma once

#include <boglfw/renderOpenGL/UniformPack.h>

// provides a uniform pack for "water-surface" uniforms from water-surface.glsl
class UPackWaterSurface : public UniformPack {
public:
	UPackWaterSurface();

	void setWaterNormalTextureSampler(int val)		{ setUniform(iTexWaterNormalSampler_, val); }

private:
	unsigned iTexWaterNormalSampler_;
};
