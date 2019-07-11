#include "ISODL_Object.h"
#include "strcat.h"

// constructs a descriptor for a simple value type property
SODL_Property_Descriptor::SODL_Property_Descriptor(SODL_Value::Type valueType)
	: isObject(false), type(valueType) {	
}

// constructs a descriptor for an object type property
SODL_Property_Descriptor::SODL_Property_Descriptor(std::string objectType)
	: isObject(true), objectType(objectType) {
}

// constructs a descriptor for a callback (std::function<void(argTypes...)>)
SODL_Property_Descriptor::SODL_Property_Descriptor(void* funcPtr, std::vector<SODL_Value::Type> argTypes)
	: isObject(false), type(SODL_Value::Type::Callback)
	, callbackArgTypes(argTypes), callbackPtr(funcPtr) {
}

// defines a "primary" (mandatory) property; primary property values can be written directly in the element's declaration header
void ISODL_Object::definePrimaryProperty(const char* name, SODL_Value::Type type) {
	primaryPropertyDesc_.push_back({type});
	mapPropertyDesc_[name] = {type};
}

// defines a "secondary" (optional) property; these must be defined as name: value within the element's block
void ISODL_Object::defineSecondaryProperty(const char* name, SODL_Property_Descriptor desc) {
	mapPropertyDesc_[name] = desc;
}

SODL_result ISODL_Object::setPrimaryProperty(unsigned index, SODL_Value const& val) {
	return SODL_result::error("not implemented");
}

SODL_result ISODL_Object::instantiateClass(std::string className, ISODL_Object* &out_pInstance) {
	return SODL_result::error("not implemented");
}

SODL_result ISODL_Object::addChildObject(ISODL_Object* &out_pInstance) {
	return SODL_result::error("not implemented");
}

SODL_result ISODL_Object::setPropertyValue(std::string propName, SODL_PropValue const& val) {
	return SODL_result::error("not implemented");
}

SODL_result ISODL_Object::describePrimaryProperty(unsigned index, SODL_Property_Descriptor &out_desc) {
	if (index >= primaryPropertyDesc_.size())
		return SODL_result::error(strcat() << "Invalid primary property index: " << index);
	out_desc = primaryPropertyDesc_[index];
	return SODL_result::OK();
}

SODL_result ISODL_Object::describeProperty(std::string const& propName, SODL_Property_Descriptor &out_desc) {
	auto it = mapPropertyDesc_.find(propName);
	if (it == mapPropertyDesc_.end())
		return SODL_result::error(strcat() << "Unknown property \"" << propName << "\"");
	out_desc = it->second;
	return SODL_result::OK();
}
	