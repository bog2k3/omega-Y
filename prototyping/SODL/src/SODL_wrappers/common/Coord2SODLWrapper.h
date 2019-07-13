#pragma once

#include "../../ISODL_Object.h"

#include <boglfw/utils/FlexibleCoordinate.h>

class Coord2SODLWrapper : public ISODL_Object {
public:
	~Coord2SODLWrapper() override {}
	Coord2SODLWrapper() {
		definePrimaryProperty("x", SODL_Value::Type::Coordinate);
		definePrimaryProperty("y", SODL_Value::Type::Coordinate);
	}

private:
	FlexCoordPair coordPair_;
};
