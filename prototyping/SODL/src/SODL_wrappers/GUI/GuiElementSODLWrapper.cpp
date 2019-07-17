#include "GuiElementSODLWrapper.h"

GuiElementSODLWrapper::GuiElementSODLWrapper() {
	loadingFinished.add(std::bind(&GuiElementSODLWrapper::onLoadingFinished, this));
}

void GuiElementSODLWrapper::onLoadingFinished() {
	// apply all common properties
	if (position_)
		element_->setPosition(position_->get());
	if (size_)
		element_->setSize(size_->get());
}

void GuiElementSODLWrapper::setupCommonProperties(std::shared_ptr<GuiBasicElement> pElement) {
	element_ = pElement;
	defineSecondaryProperty("pos", {"coord2", (std::shared_ptr<ISODL_Object>*)&position_});
	defineSecondaryProperty("size", {"coord4", (std::shared_ptr<ISODL_Object>*)&size_});
}
