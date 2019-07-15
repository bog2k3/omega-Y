#pragma once

#include "SODL_common.h"

#include <unordered_map>

class GUI_SODL_OBJ_Factory : public ISODL_Object_Factory {
public:
	SODL_result constructObject(std::string const& objType, std::shared_ptr<ISODL_Object> &outObj) override;

	GUI_SODL_OBJ_Factory();
	~GUI_SODL_OBJ_Factory();

private:

	class creatorModel {
	public:
		virtual std::shared_ptr<ISODL_Object> create() = 0;
	};

	template <class C>
	class creator : public creatorModel {
	public:
		std::shared_ptr<ISODL_Object> create() override { return std::shared_ptr<ISODL_Object>(new C()); }
	};

	std::unordered_map<std::string, creatorModel*> mapCreators_;

	template <class C>
	void addClassCreator() {
		mapCreators_[C().objectType()] = new creator<C>();
	}
};
