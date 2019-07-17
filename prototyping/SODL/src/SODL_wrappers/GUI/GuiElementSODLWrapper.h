#pragma once

#include "../../ISODL_Object.h"
#include "../common/Coord2SODLWrapper.h"

#include <boglfw/GUI/GuiBasicElement.h>

class GuiElementSODLWrapper : public ISODL_Object {
public:
	virtual std::string objectType() const override { return "guiElement"; }

	virtual ~GuiElementSODLWrapper() override = default;
	GuiElementSODLWrapper();

protected:
	void setupCommonProperties(std::shared_ptr<GuiBasicElement> pElement);

private:
	std::shared_ptr<GuiBasicElement> element_;
	std::shared_ptr<Coord2SODLWrapper> position_;
	std::shared_ptr<Coord2SODLWrapper> size_;

	void onLoadingFinished();
};