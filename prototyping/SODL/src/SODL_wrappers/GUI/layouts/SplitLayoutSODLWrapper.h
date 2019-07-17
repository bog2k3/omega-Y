#pragma once

#include "LayoutSODLWrapper.h"

#include <boglfw/GUI/SplitLayout.h>

class SplitLayoutSODLWrapper : public LayoutSODLWrapper {
public:
	const std::string objectType() const override { return "splitLayout"; }
	~SplitLayoutSODLWrapper() = default;
	SplitLayoutSODLWrapper();

	std::shared_ptr<Layout> get() const override { return layout_; }

private:
	std::shared_ptr<SplitLayout> layout_;
	int32_t direction_ = 0;
	FlexCoord offset_;
};
