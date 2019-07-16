#pragma once

#include "../../ISODL_Object.h"

#include <boglfw/utils/FlexibleCoordinate.h>

class Coord2SODLWrapper : public ISODL_Object {
public:
	const std::string objectType() const override { return "coord2"; }
	~Coord2SODLWrapper() override {}
	Coord2SODLWrapper() {
		definePrimaryProperty("x", SODL_Value::Type::Coordinate, &coordPair_.x);
		definePrimaryProperty("y", SODL_Value::Type::Coordinate, &coordPair_.y);
	}

private:
	FlexCoordPair coordPair_;
};
