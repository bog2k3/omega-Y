#include "SharedUniformPacks.h"
#include "UPackCommon.h"
#include "UPackWaterSurface.h"

std::shared_ptr<UPackCommon> SharedUniformPacks::upCommon;
std::shared_ptr<UPackWaterSurface> SharedUniformPacks::upWaterSurface;

void SharedUniformPacks::initialize() {
	upCommon.reset(new UPackCommon());
	upWaterSurface.reset(new UPackWaterSurface());
}

void SharedUniformPacks::unload() {
	upCommon.reset();
	upWaterSurface.reset();
}
