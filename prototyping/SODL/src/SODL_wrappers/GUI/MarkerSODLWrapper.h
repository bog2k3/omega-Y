#pragma once

#include "../../ISODL_Object.h"

class MarkerSODLWrapper : public ISODL_Object {
public:
	const std::string objectType() const override { return "marker"; }
	~MarkerSODLWrapper() override {}
};
