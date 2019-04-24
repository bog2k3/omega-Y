#pragma once

#include <memory>

class UPackCommon;
class UPackWaterSurface;

struct SharedUniformPacks {
	static std::shared_ptr<UPackCommon> upCommon;
	static std::shared_ptr<UPackWaterSurface> upWaterSurface;

	static void initialize();
	static void unload();
};
