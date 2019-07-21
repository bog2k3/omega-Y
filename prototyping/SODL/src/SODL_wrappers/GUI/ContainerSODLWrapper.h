#pragma once

#include "GuiElementSODLWrapper.h"
#include "LayoutSODLWrapper.h"
#include "../common/Coord2SODLWrapper.h"
#include "../common/Coord4SODLWrapper.h"

#include <boglfw/GUI/GuiContainerElement.h>

class ContainerSODLWrapper : public GuiElementSODLWrapper {
public:
	std::string objectType() const override { return "container"; }
	std::string superType() const override { return GuiElementSODLWrapper::objectType(); }

	~ContainerSODLWrapper() override {}
	ContainerSODLWrapper();

protected:
	bool addChildObject(std::shared_ptr<ISODL_Object> pObj) override;
	std::shared_ptr<ISODL_Object> clone() override;

private:
	std::shared_ptr<GuiContainerElement> container_;
	std::shared_ptr<LayoutSODLWrapper> layout_;
	std::shared_ptr<Coord4SODLWrapper> padding_;
	bool transparent_;

	std::vector<std::shared_ptr<ISODL_Object>> childNodes_;

	void onLoadingFinished();
};
