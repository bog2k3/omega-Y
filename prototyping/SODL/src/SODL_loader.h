#pragma once

/*
	SODL_loader.h

	@author: bogdan ionita <bog2k3@gmail.com>
	@date: July 7, 2019

	SODL - Simplified Object Description Language
	This class implements an object loader from a SODL file
*/

#include "ISODL_Object.h"
#include "SODL_Common.h"

#include <string>
#include <utility>

class SODL_Loader {
public:
	SODL_Loader(ISODL_Object_Factory &factory)
		: factory_(factory) {
	}

	// loads a SODL file and returns a new ISODL_Object (actual type depending on the root node's type in the file)
	SODL_result loadObject(const char* filename, ISODL_Object* &out_pObj);

	// merges a SODL file with an existing ISODL_Object provided by user; The root node in the file mustn't specify a type.
	SODL_result mergeObject(ISODL_Object &object, const char* filename);

private:
	class ParseStream;

	ISODL_Object_Factory &factory_;

	std::pair<char*, size_t> readFile(const char* fileName);

	// remove all comments and reduce all whitespace to a single ' ' char;
	// the output buffer must have at least the same size as the input buffer;
	// returns the size of the preprocessed data
	size_t preprocess(const char* input, size_t length, char* output);

	SODL_result loadObjectImpl(ParseStream &stream, ISODL_Object* &out_pObj);
	SODL_result mergeObjectImpl(ISODL_Object &object, ParseStream &stream);

	SODL_result readObjectType(ParseStream &stream, std::string &out_type);
	SODL_result instantiateObject(std::string const& objType, ISODL_Object* &out_pObj);
	SODL_result readPrimaryProps(ISODL_Object &object, ParseStream &stream);
	SODL_result readObjectBlock(ISODL_Object &object, ParseStream &stream);
};
