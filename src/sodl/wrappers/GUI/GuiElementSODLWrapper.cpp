#include "GuiElementSODLWrapper.h"

#include <cstring>

GuiElementSODLWrapper::GuiElementSODLWrapper() {
	loadingFinished.add(std::bind(&GuiElementSODLWrapper::onLoadingFinished, this));
}

void GuiElementSODLWrapper::onLoadingFinished() {
	// apply all common properties
	if (position_)
		pElement_->setPosition(position_->get());
	if (size_)
		pElement_->setSize(size_->get());
	if (minSize_)
		pElement_->setMinSize(minSize_->get());
	if (maxSize_)
		pElement_->setMaxSize(maxSize_->get());
}

void GuiElementSODLWrapper::setupCommonProperties(GuiBasicElement &element) {
	pElement_ = element.shared_from_this();
	defineSecondaryProperty("pos", {"coord2", (std::shared_ptr<ISODL_Object>&)position_});
	defineSecondaryProperty("size", {"coord2", (std::shared_ptr<ISODL_Object>&)size_});
	defineSecondaryProperty("width", callMeBack<FlexCoord>(std::bind(&GuiElementSODLWrapper::setWidth, this, std::placeholders::_1)));
	defineSecondaryProperty("height", callMeBack<FlexCoord>(std::bind(&GuiElementSODLWrapper::setHeight, this, std::placeholders::_1)));
	defineSecondaryProperty("minWidth", callMeBack<FlexCoord>(std::bind(&GuiElementSODLWrapper::setMinWidth, this, std::placeholders::_1)));
	defineSecondaryProperty("minHeight", callMeBack<FlexCoord>(std::bind(&GuiElementSODLWrapper::setMinHeight, this, std::placeholders::_1)));
	defineSecondaryProperty("maxWidth", callMeBack<FlexCoord>(std::bind(&GuiElementSODLWrapper::setMaxWidth, this, std::placeholders::_1)));
	defineSecondaryProperty("maxHeight", callMeBack<FlexCoord>(std::bind(&GuiElementSODLWrapper::setMaxHeight, this, std::placeholders::_1)));
}

bool GuiElementSODLWrapper::setWidth(FlexCoord coordVal) {
	if (!size_)
		size_ = std::make_shared<Coord2SODLWrapper>(pElement_->size());
	size_->setX(coordVal);
	return true;
}

bool GuiElementSODLWrapper::setHeight(FlexCoord coordVal) {
	if (!size_)
		size_ = std::make_shared<Coord2SODLWrapper>(pElement_->size());
	size_->setY(coordVal);
	return true;
}

bool GuiElementSODLWrapper::setMinWidth(FlexCoord coordVal) {
	if (!minSize_)
		minSize_ = std::make_shared<Coord2SODLWrapper>();
	minSize_->setX(coordVal);
	return true;
}

bool GuiElementSODLWrapper::setMinHeight(FlexCoord coordVal) {
	if (!minSize_)
		minSize_ = std::make_shared<Coord2SODLWrapper>();
	minSize_->setY(coordVal);
	return true;
}

bool GuiElementSODLWrapper::setMaxWidth(FlexCoord coordVal) {
	if (!maxSize_)
		maxSize_ = std::make_shared<Coord2SODLWrapper>();
	maxSize_->setX(coordVal);
	return true;
}

bool GuiElementSODLWrapper::setMaxHeight(FlexCoord coordVal) {
	if (!maxSize_)
		maxSize_ = std::make_shared<Coord2SODLWrapper>();
	maxSize_->setY(coordVal);
	return true;
}

void GuiElementSODLWrapper::cloneCommonPropertiesTo(GuiElementSODLWrapper &dest) {
	if (position_ != nullptr)
		dest.position_ = std::dynamic_pointer_cast<Coord2SODLWrapper>(position_->clone());
	if (size_ != nullptr)
		dest.size_ = std::dynamic_pointer_cast<Coord2SODLWrapper>(size_->clone());
	if (minSize_ != nullptr)
		dest.minSize_ = std::dynamic_pointer_cast<Coord2SODLWrapper>(minSize_->clone());
	if (maxSize_ != nullptr)
		dest.maxSize_ = std::dynamic_pointer_cast<Coord2SODLWrapper>(maxSize_->clone());
}
