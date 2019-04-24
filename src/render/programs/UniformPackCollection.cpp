#include "UniformPackCollection.h"
#include "UPackCommon.h"
#include "UPackUnderwater.h"
#include "UPackWaterSurface.h"

std::shared_ptr<UPackCommon> UniformPackCollection::upCommon;
std::shared_ptr<UPackUnderwater> UniformPackCollection::upUnderwater;
std::shared_ptr<UPackWaterSurface> UniformPackCollection::upWaterSurface;

void UniformPackCollection::initialize() {
	upCommon.reset(new UPackCommon());
	upUnderwater.reset(new UPackUnderwater());
	upWaterSurface.reset(new UPackWaterSurface());
}

void UniformPackCollection::unload() {
	upCommon.reset();
	upUnderwater.reset();
	upWaterSurface.reset();
}
