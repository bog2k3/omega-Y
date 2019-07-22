#include "ContainerSODLWrapper.h"
#include "../common/Coord2SODLWrapper.h"
#include "../common/Coord4SODLWrapper.h"

ContainerSODLWrapper::ContainerSODLWrapper()
	: container_(new GuiContainerElement()) {
	setupCommonProperties(container_);

	defineSecondaryProperty("padding", {"coord4", (std::shared_ptr<ISODL_Object>&)padding_});
	defineSecondaryProperty("layout", {"layout", (std::shared_ptr<ISODL_Object>&)layout_});
	defineSecondaryProperty("transparent", {transparent_});

	defineSupportedChildTypes({	GuiElementSODLWrapper::objectType() });

	loadingFinished.add(std::bind(&ContainerSODLWrapper::onLoadingFinished, this));
}

void ContainerSODLWrapper::onLoadingFinished() {
	if (padding_ != nullptr)
		container_->setClientArea(padding_->getTopLeft(), padding_->getBottomRight());
	if (layout_ != nullptr)
		container_->useLayout(layout_->get());
	container_->setTransparentBackground(transparent_);
}

bool ContainerSODLWrapper::addChildObject(std::shared_ptr<ISODL_Object> pObj) {
	auto pElement = std::dynamic_pointer_cast<GuiElementSODLWrapper>(pObj);
	if (!pElement || !pElement->get())
		return false;
	container_->addElement(pElement->get());
	// also keep track of the ISODL_Object interface so we can add it to cloned instances:
	childNodes_.push_back(pObj);
	return true;
}

std::shared_ptr<ISODL_Object> ContainerSODLWrapper::clone() {
	std::shared_ptr<ContainerSODLWrapper> ptr(new ContainerSODLWrapper());
	cloneCommonPropertiesTo(ptr);
	ptr->padding_ = padding_;
	ptr->layout_ = layout_;
	ptr->transparent_ = transparent_;
	// clone all child objects:
	for (auto sp : childNodes_) {
		auto clonedChild = sp->clone();
		ptr->addChildObject(clonedChild);
		clonedChild->loadingFinished.trigger();
	}

	return ptr;
}
