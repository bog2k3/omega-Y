#pragma once

#include "../../ISODL_Object.h"

#include <boglfw/utils/FlexibleCoordinate.h>

class Coord4SODLWrapper : public ISODL_Object {
public:
	~Coord4SODLWrapper() override {}
	Coord4SODLWrapper() {
		definePrimaryProperty("top", SODL_Value::Type::Coordinate);
		definePrimaryProperty("right", SODL_Value::Type::Coordinate);
		definePrimaryProperty("bottom", SODL_Value::Type::Coordinate);
		definePrimaryProperty("left", SODL_Value::Type::Coordinate);
	}

private:
	FlexCoordPair coordPairTopLeft_;
	FlexCoordPair coordPairBottomRight_;
};
