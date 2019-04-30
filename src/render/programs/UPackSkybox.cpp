#include "UPackSkybox.h"

UPackSkybox::UPackSkybox() {
	iSamplerSky_ = addUniform({"textureSky", UniformType::INT});
}
