#pragma once

#include "SODL_common.h"

#include <unordered_map>

class GUI_SODL_OBJ_Factory : public ISODL_Object_Factory {
public:
	SODL_result constructObject(std::string const& objType, std::shared_ptr<ISODL_Object> &outObj) override;
	SODL_result getTypeInfo(std::string const& typeName, SODL_ObjectTypeDescriptor &outInfo) override;

	GUI_SODL_OBJ_Factory();
	~GUI_SODL_OBJ_Factory();

private:

	class creatorModel {
	public:
		virtual ~creatorModel() = default;
		virtual std::shared_ptr<ISODL_Object> create() = 0;
	};

	template <class C>
	class creator : public creatorModel {
	public:
		std::shared_ptr<ISODL_Object> create() override { return std::shared_ptr<ISODL_Object>(new C()); }
	};

	struct classDef {
		creatorModel* creator_;
		SODL_ObjectTypeDescriptor metadata_;
	};

	std::unordered_map<std::string, classDef> mapClassDef_;

	template <class C>
	void addClassDefinitionImpl(creatorModel* creatorPtr) {
		mapClassDef_[((C*)0)->C::objectType()] = classDef {
			creatorPtr,
			{ ((C*)0)->C::superType(), creatorPtr == nullptr }
		};
	}

	template<class C>
	void addClassDefinition() {
		addClassDefinitionImpl<C>(new creator<C>());
	}

	template<class C>
	void addAbstractClassDefinition() {
		addClassDefinitionImpl<C>(nullptr);
	}
};
