#pragma once

#include <string>

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
		Number,
		String,
		Enum,
		Callback
	};
	bool isBinding;
	std::string bindingName;
	float numberVal;
	std::string stringVal;
	std::string enumVal;
};

class ISODL_Object;

struct SODL_PropValue {
	SODL_Value simpleValue;
	ISODL_Object *pObject = nullptr;
};

// implement this interface to construct your objects given a type
class ISODL_Object_Factory {
public:
	// construct an object given a type;
	// return success or error with description
	virtual SODL_result constructObject(std::string const& objType, ISODL_Object* &outObj) = 0;
};
