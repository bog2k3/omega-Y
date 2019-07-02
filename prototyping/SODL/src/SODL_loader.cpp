#include "SODL_loader.h"
#include "ISODL_Object.h"

#include <boglfw/utils/filesystem.h>

#include <fstream>

// loads a SODL file and returns a new ISODL_Object (actual type depending on the root node's type in the file)
SODL_Loader::result SODL_Loader::loadObject(const char* filename) {
	auto text = readFile(filename);
	if (!text.first || !text.second) {
		return result { false, "Unable to open SODL file", nullptr };
	}
	return loadObjectImpl(text.first, text.second);
}

// merges a SODL file with an existing ISODL_Object provided by user; The root node in the file mustn't specify a type.
SODL_Loader::result SODL_Loader::mergeObject(ISODL_Object* object, const char* filename) {
	auto text = readFile(filename);
	if (!text.first || !text.second) {
		return result { false, "Unable to open SODL file", nullptr };
	}
	return mergeObjectImpl(object, text.first, text.second);
}

std::pair<char*, size_t> SODL_Loader::readFile(const char* fileName) {
	if (!filesystem::pathExists(fileName))
		return {nullptr, 0};
	size_t length = filesystem::getFileSize(fileName);
	char* buf = new char[length + 1];
	std::ifstream file(fileName);
	file.read(buf, length);
	buf[length] = 0;
	char* preprocessBuf = new char[length + 1];
	length = preprocess(buf, length, preprocessBuf);
	delete [] buf;
	return {preprocessBuf, length};
}
// remove all comments and reduce all whitespace to a single ' ' char
size_t SODL_Loader::preprocess(const char* input, size_t length, char* output) {
	const char* end = input + length;
	const char* ptr = input;
	while (ptr < end) {

	}
}

SODL_Loader::result SODL_Loader::loadObjectImpl(const char* buf, size_t length) {
	// 1. read object type
	// 2. instantiate object
	// 3. call mergeObject on intantiated object with the rest of the buffer
}

SODL_Loader::result SODL_Loader::mergeObjectImpl(ISODL_Object* object, const char* buf, size_t length) {

}
