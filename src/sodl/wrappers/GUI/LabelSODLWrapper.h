#pragma once

#include "GuiElementSODLWrapper.h"

#include <boglfw/GUI/controls/Label.h>

class LabelSODLWrapper : public GuiElementSODLWrapper {
public:
	std::string objectType() const override { return "label"; }
	std::string superType() const override { return GuiElementSODLWrapper::objectType(); }

	~LabelSODLWrapper() override {}
	LabelSODLWrapper();

	std::shared_ptr<Label> get() const { return label_; }

protected:
	std::shared_ptr<ISODL_Object> clone() override;

private:
	std::shared_ptr<Label> label_;
	std::string text_;
	float fontSize_;
	int32_t align_ = 0;
	int32_t baseline_ = 2;

	void onLoadingFinished();
};
