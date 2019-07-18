#pragma once

#include "GuiElementSODLWrapper.h"

#include <boglfw/GUI/controls/Button.h>

#include <memory>

class ButtonSODLWrapper : public GuiElementSODLWrapper {
public:
	std::string objectType() const override { return "button"; }
	std::string superType() const override { return GuiElementSODLWrapper::objectType(); }

	~ButtonSODLWrapper() override {}
	ButtonSODLWrapper();

protected:
	std::shared_ptr<ISODL_Object> clone() override;

private:
	std::shared_ptr<Button> button_;

	bool setText(std::string text);
};
