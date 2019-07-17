#pragma once

#include "../../../ISODL_Object.h"

#include <boglfw/GUI/Layout.h>

class LayoutSODLWrapper : public ISODL_Object {
public:
	std::string objectType() const override { return "layout"; }

	~LayoutSODLWrapper() override = default;
	LayoutSODLWrapper();

	std::shared_ptr<Layout> get() const;

protected:
	bool setUserPropertyValue(const char* propName, int32_t enumVal) override;

private:
	class Impl;
	Impl *pImpl_;

	void defineFreeProps();
	void defineFillProps();
	void defineGridProps();
	void defineListProps();
	void defineSplitProps();

	void onLoadingFinished();
};
