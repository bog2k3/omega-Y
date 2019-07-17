#include "ButtonSODLWrapper.h"

#include <cstring>

ButtonSODLWrapper::ButtonSODLWrapper()
	: button_(new Button("unset")) {

	definePrimaryProperty("text", {SODL_Value::Type::String});
	definePrimaryProperty("onClick", {button_->onClick, {}});
}

bool ButtonSODLWrapper::setUserPropertyValue(const char* propName, std::string const& stringVal) {
	if (!strcmp(propName, "text")) {
		button_->setText(stringVal);
		return true;
	} else
		return false;
}
