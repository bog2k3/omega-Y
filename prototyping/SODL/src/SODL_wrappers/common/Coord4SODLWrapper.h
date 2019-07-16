#pragma once

#include "../../ISODL_Object.h"

#include <boglfw/utils/FlexibleCoordinate.h>

class Coord4SODLWrapper : public ISODL_Object {
public:
	const std::string objectType() const override { return "coord4"; }
	~Coord4SODLWrapper() override {}
	Coord4SODLWrapper() {
		definePrimaryProperty("top", SODL_Value::Type::Coordinate, &coordPairTopLeft_.y);
		definePrimaryProperty("right", SODL_Value::Type::Coordinate, &coordPairBottomRight_.x);
		definePrimaryProperty("bottom", SODL_Value::Type::Coordinate, &coordPairBottomRight_.y);
		definePrimaryProperty("left", SODL_Value::Type::Coordinate, &coordPairTopLeft_.x);
	}

private:
	FlexCoordPair coordPairTopLeft_;
	FlexCoordPair coordPairBottomRight_;
};
