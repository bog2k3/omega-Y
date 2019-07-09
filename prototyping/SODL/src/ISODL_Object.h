#pragma once

#include "SODL_common.h"

#include <vector>

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
	void* callbackPtr;
};

class ISODL_Object {
public:
	virtual ~ISODL_Object() {}

	const std::string& id() const { return id_; }

protected:
	// defines a "primary" (mandatory) property; primary property values can be written directly in the element's declaration header
	void definePrimaryProperty(const char* name, SODL_Value::Type type);

	// defines a "secondary" (optional) property; these must be defined as name: value within the element's block
	void defineSecondaryProperty(const char* name, SODL_Value::Type type);

	// defines a "callback" property (primary or secondary) with a name and array of argument types.
	// the return type of all callbacks is always "void"
	void defineCallback(bool primary, const char* name, std::vector<SODL_Value::Type> argumentTypes);

private:
	friend class SODL_Loader;

	std::string id_ = "UNIDENTIFIED";

	void setId(std::string id) { id_ = id; }
	SODL_result setPrimaryProperty(unsigned index, SODL_Value const& val);
	SODL_result instantiateClass(std::string className, ISODL_Object* &out_pInstance);
	SODL_result addChildObject(ISODL_Object* &out_pInstance);
	SODL_result createProperty(std::string propName, ISODL_Object* &out_pProperty);
	SODL_result setPropertyValue(std::string propName, ISODL_Object &propObj);

	SODL_Property_Descriptor describePrimaryProperty(unsigned index);
	SODL_Property_Descriptor describeProperty(std::string const& propName);
};
