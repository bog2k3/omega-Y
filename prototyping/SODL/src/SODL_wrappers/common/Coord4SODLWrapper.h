#pragma once

#include "../../ISODL_Object.h"

#include <boglfw/utils/FlexibleCoordinate.h>

class Coord4SODLWrapper : public ISODL_Object {
public:
	std::string objectType() const override { return "coord4"; }
	~Coord4SODLWrapper() override {}
	Coord4SODLWrapper() {
		definePrimaryProperty("top", {coordPairTopLeft_.y});
		definePrimaryProperty("right", {coordPairBottomRight_.x});
		definePrimaryProperty("bottom", {coordPairBottomRight_.y});
		definePrimaryProperty("left", {coordPairTopLeft_.x});
	}

	FlexCoordPair getTopLeft() const { return coordPairTopLeft_; }
	FlexCoordPair getBottomRight() const { return coordPairBottomRight_; }

protected:
	std::shared_ptr<ISODL_Object> clone() override {
		auto ptr = std::make_shared<Coord4SODLWrapper>();
		ptr->coordPairTopLeft_ = coordPairTopLeft_;
		ptr->coordPairBottomRight_ = coordPairBottomRight_;
		return ptr;
	}

private:
	FlexCoordPair coordPairTopLeft_;
	FlexCoordPair coordPairBottomRight_;
};
