#pragma once

#include "LayoutSODLWrapper.h"

#include <boglfw/GUI/FillLayout.h>

class FillLayoutSODLWrapper : public LayoutSODLWrapper {
public:
	const std::string objectType() const override { return "fillLayout"; }
	~FillLayoutSODLWrapper() = default;
	FillLayoutSODLWrapper();

	std::shared_ptr<Layout> get() const override { return layout_; }

private:
	std::shared_ptr<FillLayout> layout_;
};
