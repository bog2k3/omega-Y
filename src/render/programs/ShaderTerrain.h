#pragma once

#include "UPackTerrain.h"

#include <boglfw/renderOpenGL/ShaderProgram.h>

#include <memory>

class ShaderTerrain : public ShaderProgram {
public:
	ShaderTerrain();

	virtual void load();

	UPackTerrain& uniforms() { return *upackTerrain_; }

private:
	std::shared_ptr<UPackTerrain> upackTerrain_ { new UPackTerrain() };
};
