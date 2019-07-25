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

std::shared_ptr<ISODL_Object> ButtonSODLWrapper::clone() {
	std::shared_ptr<ButtonSODLWrapper> ptr(new ButtonSODLWrapper());
	cloneCommonPropertiesTo(ptr);
	ptr->button_->setText(button_->text());
	ptr->button_->onClick = button_->onClick;
	return ptr;
}
