#include "ShaderTerrain.h"
#include "SharedUniformPacks.h"
#include "UPackCommon.h"

#include <boglfw/utils/assert.h>
#include <boglfw/utils/log.h>

#include <GL/glew.h>

ShaderTerrain::ShaderTerrain() {
	useUniformPack(SharedUniformPacks::upCommon);
	useUniformPack(SharedUniformPacks::upWaterSurface);
	useUniformPack(upackTerrain_);

	defineVertexAttrib("pos", GL_FLOAT, 3);
	defineVertexAttrib("normal", GL_FLOAT, 3);
	defineVertexAttrib("color", GL_FLOAT, 4);
	defineVertexAttrib("uv1", GL_FLOAT, 4);
	defineVertexAttrib("uv2", GL_FLOAT, 4);
	defineVertexAttrib("uv3", GL_FLOAT, 2);
	defineVertexAttrib("texBlendFactor", GL_FLOAT, 4);
}

void ShaderTerrain::load() {
	if (isValid())
		return; // already loaded

	if (!ShaderProgram::load("data/shaders/terrain.vert", "data/shaders/terrain.frag", "data/shaders/watercut.geom")) {
		ERROR("Failed to load terrain shaders!");
	}
}
