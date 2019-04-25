#include "ShaderWater.h"
#include "SharedUniformPacks.h"
#include "UPackCommon.h"

#include <boglfw/utils/assert.h>
#include <boglfw/utils/log.h>

#include <GL/glew.h>

ShaderWater::ShaderWater() {
	assertDbg(SharedUniformPacks::upCommon && "Uniform pack not initialized!");
	useUniformPack(SharedUniformPacks::upCommon);
	useUniformPack(SharedUniformPacks::upWaterSurface);
	useUniformPack(upackWater_);
	defineVertexAttrib("pos", GL_FLOAT, 3);
	defineVertexAttrib("fog", GL_FLOAT, 1);
}

void ShaderWater::load() {
	if (isValid())
		return; // already loaded

	if (!ShaderProgram::load("data/shaders/water.vert", "data/shaders/water.frag")) {
		ERROR("Failed to load water shader!");
	}
}
