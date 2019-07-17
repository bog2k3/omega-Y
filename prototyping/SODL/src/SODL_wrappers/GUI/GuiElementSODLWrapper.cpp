#include "GuiElementSODLWrapper.h"

GuiElementSODLWrapper::GuiElementSODLWrapper() {
	loadingFinished.add(std::bind(&GuiElementSODLWrapper::onLoadingFinished, this));
}

void GuiElementSODLWrapper::onLoadingFinished() {
	// apply all common properties
}

void GuiElementSODLWrapper::setupCommonProperties(std::shared_ptr<GuiBasicElement> pElement) {


}
