#pragma once

#include "../../ISODL_Object.h"

class TextFieldSODLWrapper : public ISODL_Object {
public:
	const std::string objectType() const override { return "textField"; }
	~TextFieldSODLWrapper() override {}
};
