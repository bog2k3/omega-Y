#pragma once

#include "../../ISODL_Object.h"

class PictureSODLWrapper : public ISODL_Object {
public:
	const std::string objectType() const override { return "picture"; }
	~PictureSODLWrapper() override {}
};
