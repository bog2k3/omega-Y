#include "LabelSODLWrapper.h"

LabelSODLWrapper::LabelSODLWrapper()
	: label_(new Label(14, "")) {
	defineEnum("enumAlignment", {
		"left", "center", "right"
	});
	definePrimaryProperty("text", {text_});
	definePrimaryProperty("fontSize", {fontSize_});
	definePrimaryProperty("align", {"enumAlignment", align_});
}

void LabelSODLWrapper::loadingFinished() {
	label_->setText(text_);
	//label_->setFontSize(fontSize_);
}
