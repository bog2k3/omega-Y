#pragma once

#include "LayoutSODLWrapper.h"

#include <boglfw/GUI/GridLayout.h>

class GridLayoutSODLWrapper : public LayoutSODLWrapper {
public:
	const std::string objectType() const override { return "gridLayout"; }
	~GridLayoutSODLWrapper() = default;
	GridLayoutSODLWrapper();

	std::shared_ptr<Layout> get() const override { return layout_; }

private:
	std::shared_ptr<GridLayout> layout_;
};
