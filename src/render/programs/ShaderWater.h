#pragma once

#include "UPackWater.h"

#include <boglfw/renderOpenGL/ShaderProgram.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <memory>

class ShaderWater : public ShaderProgram {
public:
	ShaderWater();

	virtual void load();

	UPackWater& uniforms() { return *upackWater_; }

private:
	std::shared_ptr<UPackWater> upackWater_ { new UPackWater() };
};

struct WaterVertex {
	glm::vec3 pos;
	float fog;
};
