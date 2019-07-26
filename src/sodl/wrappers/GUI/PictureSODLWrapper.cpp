#include "PictureSODLWrapper.h"

PictureSODLWrapper::PictureSODLWrapper()
	: picture_(new Picture()) {
	setupCommonProperties(*picture_);

	defineSecondaryProperty("onStartDrag", {picture_->onStartDrag, {SODL_Value::Type::Number, SODL_Value::Type::Number}});
	defineSecondaryProperty("onEndDrag", {picture_->onEndDrag, {}});
	defineSecondaryProperty("onDrag", {picture_->onDrag, {SODL_Value::Type::Number, SODL_Value::Type::Number}});
	defineSecondaryProperty("onScroll", {picture_->onScroll, {SODL_Value::Type::Number}});
}

std::shared_ptr<ISODL_Object> PictureSODLWrapper::clone() {
	std::shared_ptr<PictureSODLWrapper> ptr(new PictureSODLWrapper());
	cloneCommonPropertiesTo(*ptr);
	return ptr;
}
