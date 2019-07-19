#pragma once

#include "GuiElementSODLWrapper.h"

#include <boglfw/GUI/controls/TextField.h>

class TextFieldSODLWrapper : public GuiElementSODLWrapper {
public:
	std::string objectType() const override { return "inputField"; }
	std::string superType() const override { return GuiElementSODLWrapper::objectType(); }

	~TextFieldSODLWrapper() override {}
	TextFieldSODLWrapper();

protected:
	std::shared_ptr<ISODL_Object> clone() override { return nullptr; }

private:
	std::shared_ptr<TextField> textField_;
	enum {
		text,
		number
	} type_ = text;

	bool setType(int32_t type);
	void defineTextProps();
	void defineNumberProps();
};
