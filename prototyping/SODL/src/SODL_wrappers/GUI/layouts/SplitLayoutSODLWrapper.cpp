#include "SplitLayoutSODLWrapper.h"

SplitLayoutSODLWrapper::SplitLayoutSODLWrapper()
	: layout_(new SplitLayout()) {
	defineEnum("enumDirection", {
		"horizontal",
		"vertical"
	});
	definePrimaryProperty("direction", {"enumDirection", direction_});
	definePrimaryProperty("offset", {offset_});
	definePrimaryProperty("splitPoint", {splitPoint_});

	auto firstNodeDesc = SODL_Property_Descriptor::multiObjType(allLayoutTypes(), (std::shared_ptr<ISODL_Object>*)&splitFirst_);
	auto secondNodeDesc = SODL_Property_Descriptor::multiObjType(allLayoutTypes(), (std::shared_ptr<ISODL_Object>*)&splitSecond_);
	defineSecondaryProperty("first", firstNodeDesc);
	defineSecondaryProperty("second", secondNodeDesc);
}
