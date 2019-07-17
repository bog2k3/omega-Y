#pragma once

#include "../../ISODL_Object.h"

#include <boglfw/GUI/controls/Button.h>

#include <memory>

class ButtonSODLWrapper : public ISODL_Object {
public:
	const std::string objectType() const override { return "button"; }
	~ButtonSODLWrapper() override {}
	ButtonSODLWrapper();

protected:
	bool setUserPropertyValue(const char* propName, std::string const& stringVal) override;

private:
	std::shared_ptr<Button> button_;
};
