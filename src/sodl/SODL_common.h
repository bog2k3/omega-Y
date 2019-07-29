#pragma once

#include <string>
#include <memory>
#include <ostream>

// result that can be evaluated as bool
// 	true means success
// 	false means error
// also provides error message in case of error
struct SODL_result {
	bool success = false;

	operator bool() const {
		return success;
	}

	SODL_result() = default;

	~SODL_result() {
		if (pInnerError_)
			delete pInnerError_;
	}

	SODL_result(SODL_result const& r) {
		operator=(r);
	}

	SODL_result(SODL_result &&r) {
		operator=(std::move(r));
	}

	SODL_result& operator=(SODL_result const& r) {
		success = r.success;
		errorMessage_ = r.errorMessage_;
		if (r.pInnerError_) {
			pInnerError_ = new SODL_result(*r.pInnerError_);
		}
		return *this;
	}

	SODL_result& operator=(SODL_result && r) {
		success = r.success;
		errorMessage_ = std::move(r.errorMessage_);
		pInnerError_ = r.pInnerError_;
		r.pInnerError_ = nullptr;
		return *this;
	}

	SODL_result wrap(std::string outerError) const {
		return SODL_result {
			false,
			outerError,
			new SODL_result(*this)
		};
	}

	const SODL_result* innerError() const {
		return pInnerError_;
	}

	static SODL_result OK() {
		return {true, "", nullptr};
	}
	static SODL_result error(std::string description) {
		return {false, description, nullptr};
	}

	static SODL_result error(std::string description, unsigned lineNr) {
		return {false, description + " (at line: " + std::to_string(lineNr) + ")", nullptr};
	}

	void addLineInfo(unsigned lineNr) {
		if (!success)
			errorMessage_ += " (at line: " + std::to_string(lineNr) + ")";
	}

	std::string toString() const {
		if (success)
			return "SODL_SUCCESS";
		else
			return std::string("SODL_ERROR: ") + errorMessage_ + (
				pInnerError_
					? std::string("\n>>> ") + pInnerError_->toString()
					: std::string(""));
	}

private:
	std::string errorMessage_;
	SODL_result* pInnerError_ = nullptr;

	SODL_result(bool s, std::string e, SODL_result* pi)
		: success(s), errorMessage_(e), pInnerError_(pi) {}
};

struct SODL_Value {
	enum class Type {
		Number,			// simple 32-bit floating point number
		Coordinate,		// coordinates can be written either as numbers or percents and are delivered as FlexibleCoordinate
		String,			// strings are delivered as std::string
		Enum,			// enums are written as enum element names, and delivered as int32
		Bool,			// bool can have literal values true or false and are delivered as a bool
		Callback		// callbacks have special handling - see SODL_Property_Descriptor
	};
	bool isBinding = false;
	std::string bindingName;
	float numberVal = 0;
	bool isPercentCoord = false;
	std::string stringVal;
	std::string enumVal;
	bool boolVal = false;

	// returns a string representation for a type enum
	static const char* typeStr(Type type) {
		switch (type) {
		case Type::Number: return "Number";
		case Type::Coordinate: return "Coordinate";
		case Type::String: return "String";
		case Type::Enum: return "Enum";
		case Type::Callback: return "Callback";
		case Type::Bool: return "Bool";
		};
		return "INVALID_TYPE";
	}
};

class ISODL_Object;

struct SODL_ObjectTypeDescriptor {
	// the name of the super object type or empty if no super type exists
	std::string superType;
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
