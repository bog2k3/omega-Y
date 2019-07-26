#include "SliderSODLWrapper.h"

SliderSODLWrapper::SliderSODLWrapper()
	: slider_(new Slider()) {
	setupCommonProperties(*slider_);

	definePrimaryProperty("label", {label_});
	definePrimaryProperty("rangeMin", {rangeMin_});
	definePrimaryProperty("rangeMax", {rangeMax_});
	definePrimaryProperty("step", {step_});
	definePrimaryProperty("value", {value_});

	defineSecondaryProperty("precision", {precision_});
	defineSecondaryProperty("divisionStep", {divisionStep_});
	defineSecondaryProperty("labelStep", {labelStep_});
	defineSecondaryProperty("onChange", {slider_->onValueChanged, {SODL_Value::Type::Number}});

	loadingFinished.add(std::bind(&SliderSODLWrapper::onLoadingFinished, this));
}

std::shared_ptr<ISODL_Object> SliderSODLWrapper::clone() {
	std::shared_ptr<SliderSODLWrapper> ptr(new SliderSODLWrapper());
	cloneCommonPropertiesTo(*ptr);
	ptr->label_ = label_;
	ptr->rangeMin_ = rangeMin_;
	ptr->rangeMax_ = rangeMax_;
	ptr->step_ = step_;
	ptr->value_ = value_;
	ptr->precision_ = precision_;
	ptr->divisionStep_ = divisionStep_;
	ptr->labelStep_ = labelStep_;
	ptr->slider_->onValueChanged = slider_->onValueChanged;
	return ptr;
}

void SliderSODLWrapper::onLoadingFinished() {
	slider_->setLabel(label_);
	slider_->setRange(rangeMin_, rangeMax_, step_);
	slider_->setDisplayStyle(divisionStep_, labelStep_, precision_);
	slider_->setValue(value_);
}
