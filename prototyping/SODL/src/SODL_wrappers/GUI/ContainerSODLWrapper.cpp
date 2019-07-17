#include "ContainerSODLWrapper.h"
#include "../common/Coord2SODLWrapper.h"
#include "../common/Coord4SODLWrapper.h"

ContainerSODLWrapper::ContainerSODLWrapper()
	: container_(new GuiContainerElement()) {
	defineSecondaryProperty("size", {"coord2", nullptr});
	defineSecondaryProperty("padding", {"coord4", nullptr});
	defineSecondaryProperty("layout", SODL_Property_Descriptor::multiObjType(
		LayoutSODLWrapper::allLayoutTypes(), (std::shared_ptr<ISODL_Object>*)&layout_));
}

bool ContainerSODLWrapper::setUserPropertyValue(const char* propName, std::shared_ptr<ISODL_Object> objPtr) {
	if (!strcmp(propName, "size"))
		container_->setSize((std::dynamic_pointer_cast<Coord2SODLWrapper>(objPtr))->get());
	else if (!strcmp(propName, "padding")) {
		auto topLeft = (std::dynamic_pointer_cast<Coord4SODLWrapper>(objPtr))->getTopLeft();
		auto bottomRight = (std::dynamic_pointer_cast<Coord4SODLWrapper>(objPtr))->getBottomRight();
		container_->setClientArea(topLeft, bottomRight);
	} else
		return false;
	return true;
}

void ContainerSODLWrapper::loadingFinished() {
	container_->useLayout(layout_->get());
}
