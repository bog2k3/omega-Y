#include "PictureSODLWrapper.h"

PictureSODLWrapper::PictureSODLWrapper()
	: picture_(new Picture()) {
	setupCommonProperties(picture_);
}
