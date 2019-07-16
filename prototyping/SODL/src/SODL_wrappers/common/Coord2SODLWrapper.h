#pragma once

#include "../../ISODL_Object.h"

#include <boglfw/utils/FlexibleCoordinate.h>

class Coord2SODLWrapper : public ISODL_Object {
public:
	const std::string objectType() const override { return "coord2"; }
	~Coord2SODLWrapper() override {}
	Coord2SODLWrapper() {
		definePrimaryProperty("x", &coordPair_.x);
		definePrimaryProperty("y", &coordPair_.y);
	}

private:
	FlexCoordPair coordPair_;
};
