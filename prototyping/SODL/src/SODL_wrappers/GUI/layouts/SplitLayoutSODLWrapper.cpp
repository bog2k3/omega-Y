#include "SplitLayoutSODLWrapper.h"

SplitLayoutSODLWrapper::SplitLayoutSODLWrapper()
	: layout_(new SplitLayout()) {
	defineEnum("direction", {
		"horizontal",
		"vertical"
	});
	definePrimaryProperty("direction", {"direction", direction_});
	definePrimaryProperty("offset", {offset_});
}
