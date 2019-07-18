#pragma once

#include "GuiElementSODLWrapper.h"

class SliderSODLWrapper : public GuiElementSODLWrapper {
public:
	std::string objectType() const override { return "slider"; }
	std::string superType() const override { return GuiElementSODLWrapper::objectType(); }

	~SliderSODLWrapper() override {}

protected:
	std::shared_ptr<ISODL_Object> clone() override;
};
