#include "GuiElementSODLWrapper.h"

#include <cstring>

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
	defineSecondaryProperty("pos", {"coord2", (std::shared_ptr<ISODL_Object>&)position_});
	defineSecondaryProperty("size", {"coord2", (std::shared_ptr<ISODL_Object>&)size_});
	defineSecondaryProperty("width", callMeBack<FlexCoord>(std::bind(&GuiElementSODLWrapper::setWidth, this, std::placeholders::_1)));
	defineSecondaryProperty("height", callMeBack<FlexCoord>(std::bind(&GuiElementSODLWrapper::setHeight, this, std::placeholders::_1)));
}

bool GuiElementSODLWrapper::setWidth(FlexCoord coordVal) {
	if (!position_)
		position_ = std::make_shared<Coord2SODLWrapper>(element_->size());
	position_->setX(coordVal);
	return true;
}

bool GuiElementSODLWrapper::setHeight(FlexCoord coordVal) {
	if (!position_)
		position_ = std::make_shared<Coord2SODLWrapper>(element_->size());
	position_->setY(coordVal);
	return true;
}

void GuiElementSODLWrapper::cloneCommonPropertiesTo(std::shared_ptr<GuiElementSODLWrapper> pDest) {
	pDest->position_ = position_;
	pDest->size_ = size_;
}
