#pragma once

#include "../../ISODL_Object.h"

class LabelSODLWrapper : public ISODL_Object {
public:
	const std::string objectType() const override { return "label"; }
	~LabelSODLWrapper() override {}
};
