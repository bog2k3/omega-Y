#pragma once

#include "../../ISODL_Object.h"
#include "../common/Coord2SODLWrapper.h"

#include <boglfw/GUI/GuiBasicElement.h>

class GuiElementSODLWrapper : public ISODL_Object {
public:
	virtual std::string objectType() const override { return "guiElement"; }

	virtual ~GuiElementSODLWrapper() override = default;
	GuiElementSODLWrapper();

	std::shared_ptr<GuiBasicElement> get() const { return pElement_; }

protected:
	void setupCommonProperties(GuiBasicElement &element);
	void cloneCommonPropertiesTo(GuiElementSODLWrapper &dest);

private:
	std::shared_ptr<GuiBasicElement> pElement_;
	std::shared_ptr<Coord2SODLWrapper> position_;
	std::shared_ptr<Coord2SODLWrapper> size_;
	std::shared_ptr<Coord2SODLWrapper> minSize_;
	std::shared_ptr<Coord2SODLWrapper> maxSize_;

	void onLoadingFinished();

	bool setWidth(FlexCoord coordVal);
	bool setHeight(FlexCoord coordVal);
	bool setMinWidth(FlexCoord coordVal);
	bool setMinHeight(FlexCoord coordVal);
	bool setMaxWidth(FlexCoord coordVal);
	bool setMaxHeight(FlexCoord coordVal);
};
