#include "ButtonSODLWrapper.h"

#include <cstring>

ButtonSODLWrapper::ButtonSODLWrapper()
	: button_(new Button("unset")) {

	setupCommonProperties(button_);

	definePrimaryProperty("text", callMeBack<std::string>(std::bind(&ButtonSODLWrapper::setText, this, std::placeholders::_1)));
	definePrimaryProperty("onClick", {button_->onClick, {}});
}

bool ButtonSODLWrapper::setText(std::string text) {
	button_->setText(text);
	return true;
}
