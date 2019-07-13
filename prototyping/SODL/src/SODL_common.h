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
		Number,			// simple floating point number
		Coordinate,		// coordinates can be written either as numbers or percents and are delivered as FlexibleCoordinate
		String,			// strings are delivered as std::string
		Enum,			// enums are written as enum element names, and delivered as int
		Callback		// callbacks have special handling - see SODL_Property_Descriptor
	};
	bool isBinding;
	std::string bindingName;
	float numberVal;
	std::string stringVal;
	std::string enumVal;
};

class ISODL_Object;

// implement this interface to construct your objects given a type
class ISODL_Object_Factory {
public:
	// construct an object given a type;
	// return success or error with description
	virtual SODL_result constructObject(std::string const& objType, std::shared_ptr<ISODL_Object> &outObj) = 0;
};
