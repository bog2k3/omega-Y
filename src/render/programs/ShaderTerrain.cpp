#include "ShaderTerrain.h"
#include "UniformPackCollection.h"
#include "UPackCommon.h"

#include <boglfw/utils/assert.h>
#include <boglfw/utils/log.h>

#include <GL/glew.h>

ShaderTerrain::ShaderTerrain() {
	assertDbg(UniformPackCollection::upCommon && "Uniform pack not initialized!");
	useUniformPack(UniformPackCollection::upCommon);
	useUniformPack(upackTerrain_);
	defineVertexAttrib("pos", GL_FLOAT, 3, sizeof(TerrainVertex), offsetof(TerrainVertex, pos));
	defineVertexAttrib("normal", GL_FLOAT, 3, sizeof(TerrainVertex), offsetof(TerrainVertex, normal));
	defineVertexAttrib("color", GL_FLOAT, 3, sizeof(TerrainVertex), offsetof(TerrainVertex, color));
	defineVertexAttrib("uv1", GL_FLOAT, 4, sizeof(TerrainVertex), offsetof(TerrainVertex, uv[0]));
	defineVertexAttrib("uv2", GL_FLOAT, 4, sizeof(TerrainVertex), offsetof(TerrainVertex, uv[2]));
	defineVertexAttrib("uv3", GL_FLOAT, 2, sizeof(TerrainVertex), offsetof(TerrainVertex, uv[4]));
	defineVertexAttrib("texBlendFactor", GL_FLOAT, 4, sizeof(TerrainVertex), offsetof(TerrainVertex, texBlendFactor));
}

void ShaderTerrain::load() {
	if (isValid())
		return; // already loaded

	if (!ShaderProgram::load("data/shaders/terrain.vert", "data/shaders/terrain.frag", "data/shaders/watercut.geom")) {
		ERROR("Failed to load terrain shaders!");
	}
}
