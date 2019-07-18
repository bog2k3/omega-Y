#pragma once

#include "GuiElementSODLWrapper.h"

class TextFieldSODLWrapper : public GuiElementSODLWrapper {
public:
	std::string objectType() const override { return "textField"; }
	std::string superType() const override { return GuiElementSODLWrapper::objectType(); }

	~TextFieldSODLWrapper() override {}

protected:
	std::shared_ptr<ISODL_Object> clone() override;
};
