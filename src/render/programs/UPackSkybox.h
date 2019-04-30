#pragma once

#include <boglfw/renderOpenGL/UniformPack.h>

class UPackSkybox : public UniformPack {
public:
	UPackSkybox();

	void setSkyboxSampler(int val) { setUniform(iSamplerSky_, val); }

private:
	int iSamplerSky_;
};
