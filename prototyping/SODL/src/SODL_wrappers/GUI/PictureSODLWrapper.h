#pragma once

#include "GuiElementSODLWrapper.h"

#include <boglfw/GUI/controls/Picture.h>

class PictureSODLWrapper : public GuiElementSODLWrapper {
public:
	std::string objectType() const override { return "picture"; }
	std::string superType() const override { return GuiElementSODLWrapper::objectType(); }

	~PictureSODLWrapper() override {}
	PictureSODLWrapper();

private:
	std::shared_ptr<Picture> picture_;
};
