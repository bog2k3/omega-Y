#pragma once

#include "../../ISODL_Object.h"

#include <boglfw/GUI/Layout.h>

class LayoutSODLWrapper : public ISODL_Object {
public:
	const std::string objectType() const override { return "layout"; }
	~LayoutSODLWrapper() override {}

	virtual std::shared_ptr<Layout> get() const = 0;
};
