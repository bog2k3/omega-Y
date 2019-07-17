#pragma once

#include "../../../ISODL_Object.h"

#include <boglfw/GUI/Layout.h>

#include <string>
#include <vector>
#include <utility>

class LayoutSODLWrapper : public ISODL_Object {
public:
	virtual ~LayoutSODLWrapper() = default;

	virtual std::shared_ptr<Layout> get() const = 0;

	static std::vector<std::pair<std::string, std::string>> allLayoutTypes() {
		return {
			{"free", "freeLayout"},
			{"fill", "fillLayout"},
			{"grid", "gridLayout"},
			{"split", "splitLayout"}
		};
	}
};
