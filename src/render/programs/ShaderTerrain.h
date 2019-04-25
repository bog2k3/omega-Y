#pragma once

#include "UPackTerrain.h"

#include <boglfw/renderOpenGL/ShaderProgram.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <memory>

class ShaderTerrain : public ShaderProgram {
public:
	ShaderTerrain();

	virtual void load();

	UPackTerrain& uniforms() { return *upackTerrain_; }

private:
	std::shared_ptr<UPackTerrain> upackTerrain_ { new UPackTerrain() };
};
