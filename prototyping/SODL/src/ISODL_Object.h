#pragma once

#include "SODL_common.h"

#include <vector>
#include <unordered_map>

struct SODL_Property_Descriptor {
	// true if property is an object type, false if it's a value type
	bool isObject;
	// the value type of the property, assuming isObject==false
	SODL_Value::Type type;
	// the object type of the property, assuming isObject==true
	std::string objectType;
	// types of arguments if the property is a callback
	std::vector<SODL_Value::Type> callbackArgTypes;
	// pointer to the std::function<...> within the object that will receive the callback binding
	void* callbackPtr = nullptr;
	
	SODL_Property_Descriptor() = default;
	
	// constructs a descriptor for a simple value type property
	SODL_Property_Descriptor(SODL_Value::Type valueType);
	
	// constructs a descriptor for an object type property
	SODL_Property_Descriptor(std::string objectType);
	
	// constructs a descriptor for a callback (std::function<void(argTypes...)>)
	SODL_Property_Descriptor(void* funcPtr, std::vector<SODL_Value::Type> argTypes);
};

class ISODL_Object {
public:
	virtual ~ISODL_Object() {}

	const std::string& id() const { return id_; }

protected:
	// defines a "primary" (mandatory) property; primary property values can be written directly in the element's declaration header
	// these can only be simple types
	void definePrimaryProperty(const char* name, SODL_Value::Type type);

	// defines a "secondary" (optional) property; these must be defined as name: value within the element's block
	// these can also be object types
	void defineSecondaryProperty(const char* name, SODL_Property_Descriptor descriptor);

private:
	friend class SODL_Loader;

	std::string id_ = "UNIDENTIFIED";
	std::vector<SODL_Property_Descriptor> primaryPropertyDesc_;
	std::unordered_map<std::string, SODL_Property_Descriptor> mapPropertyDesc_;

	void setId(std::string id) { id_ = id; }
	SODL_result setPrimaryProperty(unsigned index, SODL_Value const& val);
	SODL_result setPropertyValue(std::string propName, SODL_PropValue const& val);
	SODL_result instantiateClass(std::string className, ISODL_Object* &out_pInstance);
	SODL_result addChildObject(ISODL_Object* &out_pInstance);

	SODL_result describePrimaryProperty(unsigned index, SODL_Property_Descriptor &out_desc);
	SODL_result describeProperty(std::string const& propName, SODL_Property_Descriptor &out_desc);
};
