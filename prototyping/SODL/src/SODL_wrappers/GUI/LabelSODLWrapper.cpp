#include "LabelSODLWrapper.h"

LabelSODLWrapper::LabelSODLWrapper()
	: label_(new Label(14, "")) {

	setupCommonProperties(label_);

	defineEnum("enumAlignment", {
		"left", "center", "right"
	});
	definePrimaryProperty("text", {text_});
	definePrimaryProperty("fontSize", {fontSize_});
	definePrimaryProperty("align", {"enumAlignment", align_});

	loadingFinished.add(std::bind(&LabelSODLWrapper::onLoadingFinished, this));
}

void LabelSODLWrapper::onLoadingFinished() {
	label_->setText(text_);
	//label_->setFontSize(fontSize_);
}
