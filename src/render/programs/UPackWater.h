#pragma once

#include "SharedUniformPacks.h"
#include "UPackWaterSurface.h"


#include <boglfw/renderOpenGL/UniformPack.h>

class UPackWater : public UniformPack {
public:
	UPackWater();

	void setFoamTexSampler(int val)				{ setUniform(iTexFoam_, val); }
	void setReflectionTexSampler(int val)		{ setUniform(iTexReflection_2D_, val); }
	void setRefractionTexSampler(int val)		{ setUniform(iTexRefraction_, val); }
	void setRefractionCubeTexSampler(int val)	{ setUniform(iTexRefraction_Cube_, val); }
	void setWaterNormalTexSampler(int val)		{ SharedUniformPacks::upWaterSurface->setWaterNormalTextureSampler(val); }

private:
	int iTexFoam_;
	int iTexReflection_2D_;
	int iTexRefraction_Cube_;
	int iTexRefraction_;
};
