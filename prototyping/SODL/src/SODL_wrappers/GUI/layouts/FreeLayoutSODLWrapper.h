#pragma once

#include "LayoutSODLWrapper.h"

#include <boglfw/GUI/FreeLayout.h>

class FreeLayoutSODLWrapper : public LayoutSODLWrapper {
public:
	const std::string objectType() const override { return "freeLayout"; }
	~FreeLayoutSODLWrapper() = default;
	FreeLayoutSODLWrapper();

	std::shared_ptr<Layout> get() const override { return layout_; }

private:
	std::shared_ptr<FreeLayout> layout_;
};
