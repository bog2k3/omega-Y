#pragma once

#include "GuiElementSODLWrapper.h"
#include "LayoutSODLWrapper.h"
#include "../common/Coord2SODLWrapper.h"
#include "../common/Coord4SODLWrapper.h"

#include <boglfw/GUI/GuiContainerElement.h>

class ContainerSODLWrapper : public GuiElementSODLWrapper {
public:
	std::string objectType() const override { return "container"; }
	std::string superType() const override { return GuiElementSODLWrapper::objectType(); }

	~ContainerSODLWrapper() override {}
	ContainerSODLWrapper();

	explicit ContainerSODLWrapper(GuiContainerElement &existingContainer);

	size_t childrenCount() const override { return childNodes_.size(); }
	std::shared_ptr<ISODL_Object> nthChild(size_t n) override { return childNodes_[n]; }

	std::shared_ptr<GuiContainerElement> get() const { return container_; }

	// convenience method for retrieving a GUI element by type and id
	template<class ElemType>
	std::shared_ptr<ElemType> getElement(const char* id) {
		auto pWrap = std::dynamic_pointer_cast<GuiElementSODLWrapper>(getObjectById(id));
		if (pWrap == nullptr)
			return nullptr;
		return std::dynamic_pointer_cast<ElemType>(pWrap->get());
	}

protected:
	bool addChildObject(std::shared_ptr<ISODL_Object> pObj) override;
	std::shared_ptr<ISODL_Object> clone() override;

private:
	std::shared_ptr<GuiContainerElement> container_;
	std::shared_ptr<LayoutSODLWrapper> layout_;
	std::shared_ptr<Coord4SODLWrapper> padding_;
	bool transparent_;

	std::vector<std::shared_ptr<ISODL_Object>> childNodes_;

	void initialize();
	void onLoadingFinished();
};
