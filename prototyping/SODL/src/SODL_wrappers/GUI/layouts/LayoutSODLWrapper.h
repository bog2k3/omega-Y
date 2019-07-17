#pragma once

#include "../../../ISODL_Object.h"

#include <boglfw/GUI/Layout.h>

class LayoutSODLWrapper : public ISODL_Object {
public:
	virtual ~LayoutSODLWrapper() = default;

	virtual std::shared_ptr<Layout> get() const = 0;
};
