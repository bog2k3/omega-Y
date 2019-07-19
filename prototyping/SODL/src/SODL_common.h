#pragma once

#include <string>
#include <memory>

// result that can be evaluated as bool
// 	true means success
// 	false means error
// also provides error message in case of error
struct SODL_result {
	bool success = false;
	std::string errorMessage;

	operator bool() const {
		return success;
	}

	static SODL_result OK() {
		return {true, ""};
	}
	static SODL_result error(std::string description) {
		return {false, description};
	}
};

struct SODL_Value {
	enum class Type {
		Number,			// simple 32-bit floating point number
		Coordinate,		// coordinates can be written either as numbers or percents and are delivered as FlexibleCoordinate
		String,			// strings are delivered as std::string
		Enum,			// enums are written as enum element names, and delivered as int32
		Callback		// callbacks have special handling - see SODL_Property_Descriptor
	};
	bool isBinding = false;
	std::string bindingName;
	float numberVal = 0;
	bool isPercentCoord = false;
	std::string stringVal;
	std::string enumVal;

	// returns a string representation for a type enum
	static const char* typeStr(Type type) {
		switch (type) {
		case Type::Number: return "Number";
		case Type::Coordinate: return "Coordinate";
		case Type::String: return "String";
		case Type::Enum: return "Enum";
		case Type::Callback: return "Callback";
		};
		return "INVALID_TYPE";
	}
};

class ISODL_Object;

struct SODL_ObjectTypeDescriptor {
	// the name of the super object type or empty if no super type exists
	std::string superType;
	// a vector of derived type names (for which this is the super type)
	//std::vector<std::string> derivedTypes;
	// true if this type cannot be directly instantiated
	bool isAbstract;
};

// implement this interface to construct your objects given a type
class ISODL_Object_Factory {
public:
	// construct an object given a type;
	// return success or error with description
	virtual SODL_result constructObject(std::string const& objType, std::shared_ptr<ISODL_Object> &outObj) = 0;

	// provide type-related information;
	// return an error if the typeName doesn't correspond to a known type
	virtual SODL_result getTypeInfo(std::string const& typeName, SODL_ObjectTypeDescriptor &outInfo) = 0;
};
