#pragma once

#include "SODL_common.h"

class ISODL_Object {
public:
	virtual ~ISODL_Object() {}

protected:
	// defines a "primary" (mandatory) property; primary property values can be written directly in the element's declaration header
	void definePrimaryProperty(const char* name, SODL_Value::Type type);

	// defines a "secondary" (optional) property; these must be defined as name: value within the element's block
	void defineSecondaryProperty(const char* name, SODL_Value::Type type);

private:
	friend class SODL_Loader;

	SODL_result setPrimaryProperty(unsigned index, SODL_Value const& val);
	SODL_result instantiateClass(std::string className, ISODL_Object* &out_pInstance);
	SODL_result addChildObject(ISODL_Object* &out_pInstance);
	SODL_result createProperty(std::string propName, ISODL_Object* &out_pProperty);
	SODL_result setPropertyValue(std::string propName, ISODL_Object &propObj);
};
