#pragma once

#include "../../ISODL_Object.h"

#include <boglfw/utils/FlexibleCoordinate.h>

class Coord2SODLWrapper : public ISODL_Object {
public:
	std::string objectType() const override { return "coord2"; }
	~Coord2SODLWrapper() override {}
	Coord2SODLWrapper() {
		definePrimaryProperty("x", {coordPair_.x});
		definePrimaryProperty("y", {coordPair_.y});
	}

	Coord2SODLWrapper(FlexCoordPair val)
		: Coord2SODLWrapper() {
		coordPair_ = val;
	}

	FlexCoordPair get() const { return coordPair_; }

	void setX(FlexCoord x) { coordPair_.x = x; }
	void setY(FlexCoord y) { coordPair_.y = y; }

protected:
	std::shared_ptr<ISODL_Object> clone() override {
		return std::make_shared<Coord2SODLWrapper>(coordPair_);
	}

private:
	FlexCoordPair coordPair_;
};
