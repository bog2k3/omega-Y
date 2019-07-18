#pragma once

#include "../../ISODL_Object.h"
#include "../common/Coord2SODLWrapper.h"

#include <boglfw/GUI/GuiBasicElement.h>

class GuiElementSODLWrapper : public ISODL_Object {
public:
	virtual std::string objectType() const override { return "guiElement"; }

	virtual ~GuiElementSODLWrapper() override = default;
	GuiElementSODLWrapper();

	std::shared_ptr<GuiBasicElement> get() const { return element_; }

protected:
	void setupCommonProperties(std::shared_ptr<GuiBasicElement> pElement);
	bool setUserPropertyValue(const char* pName, FlexCoord coordVal) override;

private:
	std::shared_ptr<GuiBasicElement> element_;
	std::shared_ptr<Coord2SODLWrapper> position_;
	std::shared_ptr<Coord2SODLWrapper> size_;

	void onLoadingFinished();

	void setWidth(FlexCoord coordVal);
	void setHeight(FlexCoord coordVal);
};
