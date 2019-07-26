#include "LabelSODLWrapper.h"

LabelSODLWrapper::LabelSODLWrapper()
	: label_(new Label(14, "")) {

	setupCommonProperties(*label_);

	defineEnum("enumAlignment", {
		"left", "center", "right"
	});
	defineEnum("enumBaseline", {
		"top", "middle", "bottom"
	});
	definePrimaryProperty("text", {text_});
	definePrimaryProperty("fontSize", {fontSize_});
	definePrimaryProperty("align", {"enumAlignment", align_});
	definePrimaryProperty("baseline", {"enumBaseline", baseline_});

	loadingFinished.add(std::bind(&LabelSODLWrapper::onLoadingFinished, this));
}

void LabelSODLWrapper::onLoadingFinished() {
	label_->setText(text_);
	label_->setFontSize(fontSize_);
	label_->setAlignment((Label::ALIGNMENT)align_);
	label_->setBaseline((Label::BASELINE)baseline_);
}

std::shared_ptr<ISODL_Object> LabelSODLWrapper::clone() {
	std::shared_ptr<LabelSODLWrapper> ptr(new LabelSODLWrapper());
	cloneCommonPropertiesTo(*ptr);
	ptr->text_ = text_;
	ptr->fontSize_ = fontSize_;
	ptr->align_ = align_;
	ptr->baseline_ = baseline_;
	return ptr;
}
