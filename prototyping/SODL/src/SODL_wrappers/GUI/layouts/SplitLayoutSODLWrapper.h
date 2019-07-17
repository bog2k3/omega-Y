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
	std::shared_ptr<LayoutSODLWrapper> splitFirst_;
	std::shared_ptr<LayoutSODLWrapper> splitSecond_;
	int32_t direction_;
	FlexCoord offset_;
	float splitPoint_;
};
