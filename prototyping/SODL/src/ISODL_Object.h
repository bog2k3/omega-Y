#pragma once

#include "SODL_common.h"

#include <boglfw/utils/FlexibleCoordinate.h>
#include <boglfw/utils/Event.h>

#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>

class ISODL_Object;

// Describes a property of any simple or object type;
// if user supplies a non-null valuePtr, then this will be used to set the value directly when it is loaded from the file;
// if null is provided as [valuePtr], then the value is set by invoking the [ISODL_Object::setUserPropertyValue] method which you must override.
struct SODL_Property_Descriptor {
	SODL_Property_Descriptor() = default;

	// constructs a descriptor for a simple non-callback value type property defined by SODL_Value::Type, that will be set directly into the receiving object
	template <class PropType>
	SODL_Property_Descriptor(PropType &valuePtr);

	// constructs a descriptor for a simple non-callback value type property that will be set via ISODL_Object::setUserPropertyValue()
	SODL_Property_Descriptor(SODL_Value::Type valueType);

	// constructs a descriptor for an enumeration type (that must have been previously defined within the ISODL_Object)
	SODL_Property_Descriptor(const char* enumType, int32_t &valuePtr);

	// constructs a descriptor for an object type property;
	// if nullptr is provided as [objectPtr] the value will be set via ISODL_Object::setUserPropertyValue()
	SODL_Property_Descriptor(const char* objectType, std::shared_ptr<ISODL_Object> *objectPtr);

	// constructs a descriptor for an object type property that can accept one of multiple object types
	// 		for each element in vector: first is type alias (to be read from SODL file), second is actual object type;
	// 		this allows you to use shorter aliases for different possible object types of one property; for example:
	// 			descriptor for property1: { {"alias1", "Prop1ObjectType1"}, {...}...}
	// 			SODL file: property1: alias1 5 3 1 "somePrimaryProp"
	// 			'property1' will receive an object of type Prop1ObjectType1
	// if nullptr is provided as [objectPtr] the value will be set via ISODL_Object::setUserPropertyValue()
	SODL_Property_Descriptor(std::vector<std::pair<std::string, std::string>> objectTypes, std::shared_ptr<ISODL_Object> *objectPtr);

	// constructs a descriptor for a callback (std::function<void(argTypes...)>)
	template<class FuncType>
	SODL_Property_Descriptor(std::function<FuncType> &func, std::vector<SODL_Value::Type> argTypes);

	// constructs a descriptor for an Event that will receive a handler from an action binding
	template<class HandlerFuncType>
	SODL_Property_Descriptor(Event<HandlerFuncType> &event, std::vector<SODL_Value::Type> argTypes);

	// static method for convenient template type deduction
	static SODL_Property_Descriptor multiObjType(std::vector<std::pair<std::string, std::string>> objectTypes, std::shared_ptr<ISODL_Object> *objectPtr) {
		return SODL_Property_Descriptor { objectTypes, objectPtr };
	}

private:
	friend class SODL_Loader;
	friend class ISODL_Object;

	// true if property is an object type, false if it's a value type
	bool isObject;
	// true if the valueOrCallbackPtr points to an Event<> object, false otherwise
	bool isEvent;
	// the value type of the property, assuming isObject==false
	SODL_Value::Type type;
	// for enum type properties, this holds the name of the enum defined within ISODL_Object
	std::string enumName;
	// the possible object types of the property, assuming isObject==true;
	// if more than one value is supplied, then the first primary property read is interpreted as
	// 		the effective object type and must match one of these; then that object type is instantiated;
	// first is type alias, second is actual object type to instantiate
	std::vector<std::pair<std::string, std::string>> objectTypes;
	// types of arguments if the property is a callback
	std::vector<SODL_Value::Type> callbackArgTypes;
	// pointer to the receiving value or callback of type std::function<...> within the object that will receive the callback binding
	void* valueOrCallbackPtr = nullptr;

	std::string name;

	SODL_Property_Descriptor(SODL_Value::Type type, void* valPtr);
	SODL_Property_Descriptor(void* funcOrEventPtr, std::vector<SODL_Value::Type> argTypes, bool isEvent);
};

class ISODL_Object {
public:
	virtual ~ISODL_Object() {}

	const std::string& id() const { return id_; }

	// each class that inherits ISODL_Object must define a unique object type identifier
	virtual const std::string objectType() const = 0;

protected:
	// override this method if you need to do post-processing after the object has been fully loaded
	virtual void loadingFinished() {};

	// defines a "primary" (mandatory) property; primary property values can be written directly in the element's declaration header
	// these can only be simple types or callbacks defined by SODL_Value::Type;
	void definePrimaryProperty(const char* name, SODL_Property_Descriptor descriptor);

	// defines a "secondary" (optional) property; these must be defined as name: value within the element's block
	// these can also be object types
	void defineSecondaryProperty(const char* name, SODL_Property_Descriptor descriptor);

	// defines an enumeration type for this object;
	// properties can be defined with this enum type and they will receive as value an int32_t corresponding to the index
	//   (within the provided array) of the enum value written in the SODL file
	void defineEnum(const char* enumName, std::vector<std::string> enumLabels);

	// the following methods allow the user to receive property values via callbacks instead of them being directly set into variables;
	// override any one of these that you need;
	// return true if the set was successful, or false if anything's wrong with the received value.
	// there is one method overload for each data type that an object may receive
	virtual bool setUserPropertyValue(const char* propName, float numberVal) { return false; }
	virtual bool setUserPropertyValue(const char* propName, FlexibleCoordinate const& coordVal) { return false; }
	virtual bool setUserPropertyValue(const char* propName, std::string const& stringVal) { return false; }
	virtual bool setUserPropertyValue(const char* propName, int32_t enumVal) { return false; }
	// this method is invoked for object-type properties;
	// the actual object type that is provided can be found by calling objPtr->objectType()
	virtual bool setUserPropertyValue(const char* propName, std::shared_ptr<ISODL_Object> objPtr) { return false; }

private:
	friend class SODL_Loader;

	std::string id_ = "UNIDENTIFIED";
	std::vector<SODL_Property_Descriptor> primaryPropertyDesc_;
	std::unordered_map<std::string, SODL_Property_Descriptor> mapPropertyDesc_;
	std::unordered_map<std::string, std::vector<std::string>> userEnums_;

	void setId(std::string id) { id_ = id; }
	SODL_result setPrimaryProperty(unsigned index, SODL_Value const& val);
	SODL_result setProperty(std::string const& propName, SODL_Value const& val);
	SODL_result setProperty(std::string const& propName, std::shared_ptr<ISODL_Object> objPtr);
	SODL_result instantiateClass(std::string const& className, std::shared_ptr<ISODL_Object> &out_pInstance);
	SODL_result addChildObject(std::shared_ptr<ISODL_Object> pObj);

	SODL_result describePrimaryProperty(unsigned index, SODL_Property_Descriptor &out_desc);
	SODL_result describeProperty(std::string const& propName, SODL_Property_Descriptor &out_desc);
	SODL_result resolveEnumValue(std::string const& enumName, std::string const& identName, int32_t &out_val);
};

#include "ISODL_Object_private.h"
