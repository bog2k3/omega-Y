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

std::shared_ptr<ISODL_Object> LabelSODLWrapper::clone() {
	std::shared_ptr<LabelSODLWrapper> ptr(new LabelSODLWrapper());
	cloneCommonPropertiesTo(ptr);
	ptr->text_ = text_;
	ptr->fontSize_ = fontSize_;
	ptr->align_ = align_;
	return ptr;
}
