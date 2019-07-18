#include "PictureSODLWrapper.h"

PictureSODLWrapper::PictureSODLWrapper()
	: picture_(new Picture()) {
	setupCommonProperties(picture_);
}

std::shared_ptr<ISODL_Object> PictureSODLWrapper::clone() {
	std::shared_ptr<PictureSODLWrapper> ptr(new PictureSODLWrapper());
	cloneCommonPropertiesTo(ptr);
	return ptr;
}
