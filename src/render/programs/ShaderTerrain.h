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

struct TerrainVertex {
	static const unsigned nTextures = 5;

	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv[nTextures];	// uvs for each texture layer
	glm::vec4 texBlendFactor;	// 4 texture blend factors: x is between grass1 & grass2,
								// 							y between rock1 & rock2
								//							z between grass/sand and rock (highest priority)
								//							w between grass and sand
};
