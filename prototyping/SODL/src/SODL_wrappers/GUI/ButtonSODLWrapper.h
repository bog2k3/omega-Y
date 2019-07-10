#pragma once

#include "../../ISODL_Object.h"

#include <boglfw/GUI/controls/Button.h>

#include <memory>

class ButtonSODLWrapper : public ISODL_Object {
public:
	~ButtonSODLWrapper() override {}
	ButtonSODLWrapper();

private:
	std::shared_ptr<Button> button_;
};
