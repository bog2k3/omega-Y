#pragma once

#include "LayoutSODLWrapper.h"

#include <boglfw/GUI/ListLayout.h>

class ListLayoutSODLWrapper : public LayoutSODLWrapper {
public:
	const std::string objectType() const override { return "listLayout"; }
	~ListLayoutSODLWrapper() = default;
	ListLayoutSODLWrapper();

	std::shared_ptr<Layout> get() const override { return layout_; }

private:
	std::shared_ptr<ListLayout> layout_;
};
