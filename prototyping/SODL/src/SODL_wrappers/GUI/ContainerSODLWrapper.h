#pragma once

#include "../../ISODL_Object.h"
#include "layouts/LayoutSODLWrapper.h"

#include <boglfw/GUI/GuiContainerElement.h>

class ContainerSODLWrapper : public ISODL_Object {
public:
	const std::string objectType() const override { return "container"; }
	~ContainerSODLWrapper() override {}
	ContainerSODLWrapper();

protected:
	void loadingFinished() override;
	bool setUserPropertyValue(const char* propName, std::shared_ptr<ISODL_Object> objPtr) override;

private:
	std::shared_ptr<GuiContainerElement> container_;
	std::shared_ptr<LayoutSODLWrapper> layout_;
};
