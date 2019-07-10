#include "ISODL_Object.h"

// defines a "primary" (mandatory) property; primary property values can be written directly in the element's declaration header
void ISODL_Object::definePrimaryProperty(const char* name, SODL_Value::Type type) {

}

// defines a "secondary" (optional) property; these must be defined as name: value within the element's block
void ISODL_Object::defineSecondaryProperty(const char* name, SODL_Property_Descriptor) {

}

SODL_result ISODL_Object::setPrimaryProperty(unsigned index, SODL_Value const& val) {

}

SODL_result ISODL_Object::instantiateClass(std::string className, ISODL_Object* &out_pInstance) {

}

SODL_result ISODL_Object::addChildObject(ISODL_Object* &out_pInstance) {

}

SODL_result ISODL_Object::setPropertyValue(std::string propName, SODL_PropValue const& val) {

}

SODL_Property_Descriptor ISODL_Object::describePrimaryProperty(unsigned index) {
	return SODL_Property_Descriptor {};
}

SODL_Property_Descriptor describeProperty(std::string const& propName) {
	return SODL_Property_Descriptor {};
}
	