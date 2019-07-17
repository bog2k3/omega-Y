#pragma once

#include "GuiElementSODLWrapper.h"

class PictureSODLWrapper : public GuiElementSODLWrapper {
public:
	std::string objectType() const override { return "picture"; }
	std::string superType() const override { return GuiElementSODLWrapper::objectType(); }

	~PictureSODLWrapper() override {}
};
