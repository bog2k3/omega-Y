#pragma once

#include <memory>

class UPackCommon;
class UPackUnderwater;
class UPackWaterSurface;

struct UniformPackCollection {
	static std::shared_ptr<UPackCommon> upCommon;
	static std::shared_ptr<UPackUnderwater> upUnderwater;
	static std::shared_ptr<UPackWaterSurface> upWaterSurface;

	static void initialize();
	static void unload();
};
