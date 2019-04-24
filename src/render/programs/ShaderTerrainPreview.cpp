#include "ShaderTerrainPreview.h"

#include <boglfw/utils/log.h>

void ShaderTerrainPreview::load() {
	if (isValid())
		return; // already loaded
	if (!ShaderProgram::load("data/shaders/terrain-preview.vert", "data/shaders/terrain-preview.frag")) {
		ERROR("Failed to load terrain shaders!");
	}
}
