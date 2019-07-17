#pragma once

#include "../../ISODL_Object.h"

#include <boglfw/GUI/controls/Label.h>

class LabelSODLWrapper : public ISODL_Object {
public:
	const std::string objectType() const override { return "label"; }
	~LabelSODLWrapper() override {}
	LabelSODLWrapper();

	std::shared_ptr<Label> get() const { return label_; }

protected:
	void loadingFinished() override;

private:
	std::shared_ptr<Label> label_;
	std::string text_;
	float fontSize_;
	int32_t align_;
};
