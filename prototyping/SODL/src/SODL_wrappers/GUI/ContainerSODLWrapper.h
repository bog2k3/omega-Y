#pragma once

#include "../../ISODL_Object.h"

#include <boglfw/GUI/GuiContainerElement.h>

class ContainerSODLWrapper : public ISODL_Object {
public:
	~ContainerSODLWrapper() override {}
	ContainerSODLWrapper();
	
private:
	std::shared_ptr<GuiContainerElement> container_;
};
