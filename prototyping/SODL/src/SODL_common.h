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

};
