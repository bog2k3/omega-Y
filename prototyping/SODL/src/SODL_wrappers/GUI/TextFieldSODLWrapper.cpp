#include "TextFieldSODLWrapper.h"

TextFieldSODLWrapper::TextFieldSODLWrapper()
	: textField_(new TextField()) {
	setupCommonProperties(textField_);

	defineEnum("enumType", {"text", "number"});
	definePrimaryProperty("type", {"enumType", std::bind(&TextFieldSODLWrapper::setType, this, std::placeholders::_1)});
	defineSecondaryProperty("onChange", {textField_->onChanged, {}});
}

bool TextFieldSODLWrapper::setType(int32_t type) {
	type_ = (decltype(type_))(type);
	switch (type_) {
		case text:
			defineTextProps();
			return true;
		case number:
			defineNumberProps();
			return true;
		default:
			return false;
	}
	return true;
}

void TextFieldSODLWrapper::defineTextProps() {
	definePrimaryProperty("value", {textValue_});
}

void TextFieldSODLWrapper::defineNumberProps() {
	definePrimaryProperty("value", {numberValue_});
}
