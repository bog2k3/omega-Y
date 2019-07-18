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
	defineSecondaryProperty("pos", {"coord2", (std::shared_ptr<ISODL_Object>*)&position_});
	defineSecondaryProperty("size", {"coord4", (std::shared_ptr<ISODL_Object>*)&size_});
	defineSecondaryProperty("width", {SODL_Value::Type::Coordinate});
	defineSecondaryProperty("height", {SODL_Value::Type::Coordinate});
}

bool GuiElementSODLWrapper::setUserPropertyValue(const char* pName, FlexCoord coordVal) {
	if (!strcmp(pName, "width")) {
		setWidth(coordVal);
		return true;
	} else if (!strcmp(pName, "height")) {
		setHeight(coordVal);
		return true;
	} else
		return false;
}

void GuiElementSODLWrapper::setWidth(FlexCoord coordVal) {
	if (!position_)
		position_ = std::make_shared<Coord2SODLWrapper>(element_->size());
	position_->setX(coordVal);
}

void GuiElementSODLWrapper::setHeight(FlexCoord coordVal) {
	if (!position_)
		position_ = std::make_shared<Coord2SODLWrapper>(element_->size());
	position_->setY(coordVal);
}
