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
	SODL_Property_Descriptor(SODL_Property_Descriptor const&) = default;
	SODL_Property_Descriptor(SODL_Property_Descriptor &&) = default;
	SODL_Property_Descriptor& operator=(SODL_Property_Descriptor const&) = default;
	SODL_Property_Descriptor& operator=(SODL_Property_Descriptor &&) = default;

	// constructs a descriptor for a simple non-callback value type property defined by SODL_Value::Type, that will be set directly into the receiving object
	template <class PropType, class = typename std::enable_if<!std::is_same<SODL_Property_Descriptor, PropType>::value, void*>::type>
	SODL_Property_Descriptor(PropType &valuePtr, ...);

	// constructs a descriptor for an enumeration type (that must have been previously defined within the ISODL_Object);
	SODL_Property_Descriptor(const char* enumType, int32_t &valuePtr);

	// constructs a descriptor for a simple non-callback, value type property that will be set via the provided callback;
	// [setValueCallback] will be invoked with the value read from the file; return true for success, false if there's a problem with the value
	template <class PropType>
	SODL_Property_Descriptor(std::function<bool(PropType)> setValueCallback);

	// constructs a descriptor for an enum value that will be set via the provided callback
	// A valid enum name must be provided as the first argument;
	// [setValueCallback] will be invoked with the value read from the file; return true for success, false if there's a problem with the value
	SODL_Property_Descriptor(const char* enumType, std::function<bool(int32_t)> setValueCallback);

	// constructs a descriptor for an object type property;
	SODL_Property_Descriptor(const char* objectType, std::shared_ptr<ISODL_Object> &objectPtr);

	// constructs a descriptor for an object type property that will be set via the provided callback
	SODL_Property_Descriptor(const char* objectType, std::function<bool(std::shared_ptr<ISODL_Object>)> setObjPropCallback);

	// constructs a descriptor for a callback (std::function<void(argTypes...)>)
	template<class FuncType>
	SODL_Property_Descriptor(std::function<FuncType> &func, std::vector<SODL_Value::Type> argTypes);

	// constructs a descriptor for an Event that will receive a handler from an action binding
	template<class HandlerFuncType>
	SODL_Property_Descriptor(Event<HandlerFuncType> &event, std::vector<SODL_Value::Type> argTypes);

private:
	friend class SODL_Loader;
	friend class ISODL_Object;

	class CallbackWrapperModel;

	// true if property is an object type, false if it's a value type
	bool isObject;
	// true if the valueOrCallbackPtr points to an Event<> object, false otherwise
	bool isEvent;
	// the value type of the property, assuming isObject==false
	SODL_Value::Type type;
	// for enum type properties, this holds the name of the enum defined within ISODL_Object
	std::string enumName;
	// the object type of the property, assuming isObject==true;
	std::string objectType;
	// types of arguments if the property is a callback
	std::vector<SODL_Value::Type> callbackArgTypes;
	// pointer to the receiving value or callback of type std::function<...> within the object that will receive the callback binding
	void* valueOrCallbackPtr = nullptr;

	std::shared_ptr<CallbackWrapperModel> pCallbackWrapper;

	std::string name;

	SODL_Property_Descriptor(SODL_Value::Type type, void* valPtr);
	SODL_Property_Descriptor(void* funcOrEventPtr, std::vector<SODL_Value::Type> argTypes, bool isEvent);
};

class ISODL_Object {
public:
	virtual ~ISODL_Object() {}

	const std::string& id() const { return id_; }

	// each class that inherits ISODL_Object must define a unique object type identifier
	virtual std::string objectType() const = 0;

	// override this to provide your object's super type (the object type of the ISODL_Object that represents the superclass)
	virtual std::string superType() const { return ""; }

	// implement this method to return a clone of your object along with all properties and children for class instantiation
	virtual std::shared_ptr<ISODL_Object> clone() = 0;

	// subscribe to this event if you need to do post-processing after the object has been fully loaded
	Event<void()> loadingFinished;

protected:
	// defines a "primary" (mandatory) property; primary property values can be written directly in the element's declaration header
	// these can only be simple types or callbacks defined by SODL_Value::Type;
	void definePrimaryProperty(const char* name, SODL_Property_Descriptor descriptor);

	// defines a "secondary" (optional) property; these must be defined as name: value within the element's block
	// these can also be object types
	void defineSecondaryProperty(const char* name, SODL_Property_Descriptor descriptor);

	// convenience method to construct a property descriptor for a property that is assigned via a callback
	template<class PropType>
	SODL_Property_Descriptor callMeBack(std::function<bool(PropType)> cb) {
		return SODL_Property_Descriptor(cb);
	}

	// defines an enumeration type for this object;
	// properties can be defined with this enum type and they will receive as value an int32_t corresponding to the index
	//   (within the provided array) of the enum value written in the SODL file
	void defineEnum(const char* enumName, std::vector<std::string> enumLabels);

	// supply a vector of object type names that this object supports as children;
	// if none is supplied, then no children can be defined in the SODL file for this object
	void defineSupportedChildTypes(std::vector<std::string> childTypes);

	// override this method to receive child objects of the supported types defined previously
	virtual bool addChildObject(std::shared_ptr<ISODL_Object> pObj) { return false; }

private:
	friend class SODL_Loader;

	std::string id_ = "UNIDENTIFIED";
	std::vector<SODL_Property_Descriptor> primaryPropertyDesc_;
	std::unordered_map<std::string, SODL_Property_Descriptor> mapPropertyDesc_;
	std::unordered_map<std::string, std::vector<std::string>> userEnums_;
	std::vector<std::string> childTypes_;

	void setId(std::string id) { id_ = id; }
	SODL_result setPrimaryProperty(unsigned index, SODL_Value const& val);
	SODL_result setProperty(std::string const& propName, SODL_Value const& val);
	SODL_result setProperty(std::string const& propName, std::shared_ptr<ISODL_Object> objPtr);

	SODL_result describePrimaryProperty(unsigned index, SODL_Property_Descriptor &out_desc);
	SODL_result describeProperty(std::string const& propName, SODL_Property_Descriptor &out_desc);
	SODL_result resolveEnumValue(std::string const& enumName, std::string const& identName, int32_t &out_val);
	bool supportsChildType(std::string const& objType);
};

#include "ISODL_Object_private.h"
