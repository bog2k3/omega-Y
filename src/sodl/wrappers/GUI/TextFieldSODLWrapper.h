#pragma once

#include "GuiElementSODLWrapper.h"

#include <boglfw/GUI/controls/TextField.h>

class TextFieldSODLWrapper : public GuiElementSODLWrapper {
public:
	std::string objectType() const override { return "inputField"; }
	std::string superType() const override { return GuiElementSODLWrapper::objectType(); }

	~TextFieldSODLWrapper() override {}
	TextFieldSODLWrapper();

	std::shared_ptr<TextField> get() const { return textField_; }

protected:
	std::shared_ptr<ISODL_Object> clone() override;

private:
	std::shared_ptr<TextField> textField_;
	enum : int32_t {
		text,
		number
	} type_ = text;
	std::string textValue_;
	float numberValue_;

	bool setType(int32_t type);
	void defineTextProps();
	void defineNumberProps();
	void onLoadingFinished();
};
