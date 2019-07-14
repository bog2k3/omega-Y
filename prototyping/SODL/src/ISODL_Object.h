#pragma once

#include "SODL_common.h"

#include <vector>
#include <unordered_map>
#include <memory>

struct SODL_Property_Descriptor {
	// true if property is an object type, false if it's a value type
	bool isObject;
	// the value type of the property, assuming isObject==false
	SODL_Value::Type type;
	// the possible object types of the property, assuming isObject==true
	// if more than one value is supplied, then the first primary property read is interpreted as
	// the effective object type and must match one of these; then that object type is instantiated;
	// first is type alias, second is actual object type to instantiate
	std::vector<std::pair<std::string, std::string>> objectTypes;
	// types of arguments if the property is a callback
	std::vector<SODL_Value::Type> callbackArgTypes;
	// pointer to the std::function<...> within the object that will receive the callback binding
	void* callbackPtr = nullptr;
	
	SODL_Property_Descriptor() = default;
	
	// constructs a descriptor for a simple value type property
	SODL_Property_Descriptor(SODL_Value::Type valueType);
	
	// constructs a descriptor for an object type property
	SODL_Property_Descriptor(std::string objectType);
	
	// constructs a descriptor for an object type property that can accept one of multiple object types
	// for each element in vector: first is type alias (to be read from SODL file), second is actual object type
	// this allows you to use shorter aliases for different possible object types of one property; for example:
	// descriptor for property1: { {"alias1", "Prop1ObjectType1"}, {...}...} 
	// SODL file: property1: alias1 5 3 1 "somePrimaryProp"
	// property1 will receive an object of type Prop1ObjectType1
	SODL_Property_Descriptor(std::vector<std::pair<std::string, std::string>> objectTypes);
	
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
	SODL_result setProperty(std::string const& propName, SODL_Value const& val);
	SODL_result setProperty(std::string const& propName, std::string const& objectType, std::shared_ptr<ISODL_Object> objPtr);
	SODL_result instantiateClass(std::string const& className, std::shared_ptr<ISODL_Object> &out_pInstance);
	SODL_result addChildObject(std::shared_ptr<ISODL_Object> pObj);

	SODL_result describePrimaryProperty(unsigned index, SODL_Property_Descriptor &out_desc);
	SODL_result describeProperty(std::string const& propName, SODL_Property_Descriptor &out_desc);
};
