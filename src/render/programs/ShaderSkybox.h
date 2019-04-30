#pragma once

#include "UPackSkybox.h"
#include <boglfw/renderOpenGL/ShaderProgram.h>

#include <memory>

class ShaderSkybox : public ShaderProgram {
public:
	ShaderSkybox();

	virtual void load();

	UPackSkybox& uniforms() { return *upackSkybox_; }

private:
	std::shared_ptr<UPackSkybox> upackSkybox_ { new UPackSkybox() };
};
