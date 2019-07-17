#include "ContainerSODLWrapper.h"

ContainerSODLWrapper::ContainerSODLWrapper()
	: container_(new GuiContainerElement()) {
	defineSecondaryProperty("size", {"coord2", nullptr});
	defineSecondaryProperty("padding", {"coord4", nullptr});
	defineSecondaryProperty("layout", SODL_Property_Descriptor::multiObjType({
		{"free", "freeLayout"},
		{"fill", "fillLayout"},
		{"grid", "gridLayout"},
		{"split", "splitLayout"}
	}, (std::shared_ptr<ISODL_Object>*)&layout_));
}

void ContainerSODLWrapper::loadingFinished() {
	container_->useLayout(layout_->get());
}
