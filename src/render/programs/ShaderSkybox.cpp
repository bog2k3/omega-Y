#include "ShaderSkybox.h"
#include "SharedUniformPacks.h"
#include "UPackCommon.h"

#include <boglfw/utils/log.h>

#include <GL/glew.h>

ShaderSkybox::ShaderSkybox() {
	useUniformPack(SharedUniformPacks::upCommon);
	useUniformPack(upackSkybox_);

	defineVertexAttrib("pos", GL_FLOAT, 3);
}

void ShaderSkybox::load() {
	if (isValid())
		return; // already loaded

	if (!ShaderProgram::load("data/shaders/skybox.vert", "data/shaders/skybox.frag")) {
		ERROR("Failed to load skybox shader!");
	}
}
