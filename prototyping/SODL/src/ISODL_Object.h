#pragma once

#include "SODL_common.h"

class ISODL_Object {
public:
	SODL_result setPrimaryProperty(unsigned index, SODL_Value const& val);
	SODL_result instantiateClass(std::string className, ISODL_Object* &out_pInstance);
	SODL_result addChildObject(ISODL_Object* &out_pInstance);
	SODL_result createProperty(std::string propName, ISODL_Object* &out_pProperty);
	SODL_result setPropertyValue(std::string propName, ISODL_Object &propObj);
};
