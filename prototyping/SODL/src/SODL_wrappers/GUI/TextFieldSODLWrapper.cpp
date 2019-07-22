#include "TextFieldSODLWrapper.h"

TextFieldSODLWrapper::TextFieldSODLWrapper()
	: textField_(new TextField()) {
	setupCommonProperties(textField_);

	defineEnum("enumType", {"text", "number"});
	definePrimaryProperty("type", {"enumType", std::bind(&TextFieldSODLWrapper::setType, this, std::placeholders::_1)});
	defineSecondaryProperty("onChange", {textField_->onChanged, {}});

	loadingFinished.add(std::bind(&TextFieldSODLWrapper::onLoadingFinished, this));
}

bool TextFieldSODLWrapper::setType(int32_t type) {
	type_ = (decltype(type_))(type);
	switch (type_) {
		case text:
			defineTextProps();
			break;
		case number:
			defineNumberProps();
			break;
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

void TextFieldSODLWrapper::onLoadingFinished() {
	textField_->setType((TextField::Type)type_);
	if (type_ == text)
		textField_->setText(textValue_);
	else
		textField_->setValue(numberValue_);
}
