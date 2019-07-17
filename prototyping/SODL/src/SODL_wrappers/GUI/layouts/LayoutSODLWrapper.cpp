#include "LayoutSODLWrapper.h"

#include <boglfw/GUI/FillLayout.h>
#include <boglfw/GUI/FreeLayout.h>
#include <boglfw/GUI/GridLayout.h>
#include <boglfw/GUI/ListLayout.h>
#include <boglfw/GUI/SplitLayout.h>

#include <cstring>

class LayoutSODLWrapper::Impl {
public:
	enum layoutType : int32_t {
		fill,
		free,
		grid,
		list,
		split
	} type;

	bool isSet = false;
	std::shared_ptr<Layout> pLayout;

	struct {
		std::shared_ptr<FillLayout> ptr;
	} fillProps;

	struct {
		std::shared_ptr<FreeLayout> ptr;
	} freeProps;

	struct {
		std::shared_ptr<GridLayout> ptr;
	} gridProps;

	struct {
		std::shared_ptr<ListLayout> ptr;
	} listProps;

	struct {
		std::shared_ptr<SplitLayout> ptr;
		std::shared_ptr<LayoutSODLWrapper> splitFirst;
		std::shared_ptr<LayoutSODLWrapper> splitSecond;
		int32_t direction;
		FlexCoord offset;
		float splitPoint;
	} splitProps;

	void create() {
		switch (type) {
		case fill:
			pLayout = fillProps.ptr = std::make_shared<FillLayout>();
			break;
		case free:
			pLayout = freeProps.ptr = std::make_shared<FreeLayout>();
			break;
		case grid:
			pLayout = gridProps.ptr = std::make_shared<GridLayout>();
			break;
		case list:
			pLayout = listProps.ptr = std::make_shared<ListLayout>();
			break;
		case split:
			pLayout = splitProps.ptr = std::make_shared<SplitLayout>();
			break;
		default:
			assertDbg(false && "invalid layout type");
		}
		isSet = true;
	};

	void applyProps() {

	}
};

LayoutSODLWrapper::LayoutSODLWrapper()
	: pImpl_(new Impl()) {
	defineEnum("enumLayoutType", {
		"fill",
		"free",
		"grid",
		"list",
		"split"
	});
	definePrimaryProperty("type", {"enumLayoutType", nullptr});
	// the rest of the properties are defined dynamically depending on this first one
	// see setUserPropertyValue

	loadingFinished.add(std::bind(&LayoutSODLWrapper::onLoadingFinished, this));
}

bool LayoutSODLWrapper::setUserPropertyValue(const char* propName, int32_t enumVal) {
	if (!strcmp(propName, "type")) {
		if (pImpl_->isSet)
			return false; // type already set, we only allow it once
		pImpl_->type = (Impl::layoutType)enumVal;
		pImpl_->create();
		switch (pImpl_->type) {
			case Impl::fill:
				defineFillProps();
				break;
			case Impl::free:
				defineFreeProps();
				break;
			case Impl::grid:
				defineGridProps();
				break;
			case Impl::list:
				defineListProps();
				break;
			case Impl::split:
				defineSplitProps();
				break;
		}
		return true;
	} else
		return false;
}

void LayoutSODLWrapper::defineFillProps() {

}

void LayoutSODLWrapper::defineFreeProps() {

}

void LayoutSODLWrapper::defineGridProps() {

}

void LayoutSODLWrapper::defineListProps() {

}

void LayoutSODLWrapper::defineSplitProps() {
	defineEnum("enumDirection", {
		"horizontal",
		"vertical"
	});
	definePrimaryProperty("direction", {"enumDirection", pImpl_->splitProps.direction});
	definePrimaryProperty("offset", {pImpl_->splitProps.offset});
	definePrimaryProperty("splitPoint", {pImpl_->splitProps.splitPoint});

	defineSecondaryProperty("first", {"layout", (std::shared_ptr<ISODL_Object>*)&pImpl_->splitProps.splitFirst});
	defineSecondaryProperty("second", {"layout", (std::shared_ptr<ISODL_Object>*)&pImpl_->splitProps.splitSecond});
}

std::shared_ptr<Layout> LayoutSODLWrapper::get() const {
	return pImpl_->pLayout;
}

void LayoutSODLWrapper::onLoadingFinished() {
	pImpl_->applyProps();
}
