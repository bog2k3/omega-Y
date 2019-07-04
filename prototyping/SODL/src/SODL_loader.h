#pragma once

/*
	SODL_loader.h

	@author: bogdan ionita <bog2k3@gmail.com>
	@date: July 7, 2019

	SODL - Simplified Object Description Language
	This class implements an object loader from a SODL file
*/

#include "ISODL_Object.h"

#include <string>
#include <utility>

class SODL_Loader {
public:
	SODL_Loader() = default;

	struct result {
		bool success = false;
		std::string errorMessage;

		ISODL_Object *rootObject = nullptr;

		operator bool() const {
			return success;
		}
	};

	// loads a SODL file and returns a new ISODL_Object (actual type depending on the root node's type in the file)
	result loadObject(const char* filename);

	// merges a SODL file with an existing ISODL_Object provided by user; The root node in the file mustn't specify a type.
	result mergeObject(ISODL_Object* object, const char* filename);

private:
	class ParseStream;

	std::pair<char*, size_t> readFile(const char* fileName);

	// remove all comments and reduce all whitespace to a single ' ' char;
	// the output buffer must have at least the same size as the input buffer;
	// returns the size of the preprocessed data
	size_t preprocess(const char* input, size_t length, char* output);

	result loadObjectImpl(ParseStream &stream);
	result mergeObjectImpl(ISODL_Object* object, ParseStream &stream);

	result readObjectType(ParseStream &stream, std::string &out_type);
	result instantiateObject(std::string const& objType);
	result readPrimaryProps(ISODL_Object* object, ParseStream &stream);
	result readObjectBlock(ISODL_Object* object, ParseStream &stream);
};
