#include "SODL_loader.h"
#include "ISODL_Object.h"

#include <boglfw/utils/filesystem.h>
#include <boglfw/utils/assert.h>

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
	std::ofstream f(std::string(filename) + ".out.txt");
	f << text.first;
	f.close();
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
	auto isWhitespace = [](const char c) {
		return c == ' ' || c == '\t' || c == '\r';
	};
	auto isEOL = [](const char c) {
		return c == '\n';
	};
	auto commentStarts = [](const char *c, const char *end) {
		return c+1 < end && *c == '/' && *(c+1)=='/';
	};

	char* outputStart = output;
	const char* end = input + length;
	const char* ptr = input;
	while (ptr < end) {
		// process current line
		// 1. skip leading whitespace
		while (ptr < end && isWhitespace(*ptr))
			ptr++;
		// we're now either at the end of file or at the first non-white-space character on the line
		const char* lineStart = ptr;
		if (ptr == end)
			break;
		// if the line was empty, skip it
		if (isEOL(*ptr)) {
			ptr++;
			continue;
		}
		bool previousWhitespace = false;
		while (ptr < end && !isEOL(*ptr) && !commentStarts(ptr, end)) {
			// we want to reduce all whitespaces to a single character
			while (ptr < end && isWhitespace(*ptr) && previousWhitespace)
				ptr++;
			if (ptr == end || commentStarts(ptr, end))
				break;
			previousWhitespace = isWhitespace(*ptr);
			*output = previousWhitespace ? ' ' : *ptr, output++, ptr++;
		}
		// check and skip comments
		const char* commentBegining = nullptr;
		if (commentStarts(ptr, end)) {
			commentBegining = ptr;
			while (ptr < end && !isEOL(*ptr))
				ptr++;
			if (ptr == end)
				break;
		}
		assertDbg(ptr == end || isEOL(*ptr));
		//bool lastOutputIsEOL = output > outputStart && isEOL(*(output-1));
		if (ptr != end && isEOL(*ptr)) {
			if (commentBegining != lineStart) {
				// if there was something on the line before the comment, we also write the EOL char
				*output = *ptr, output++, ptr++;
			} else {
				ptr++;
			}
			//if (!lastOutputIsEOL)
			//	*output = *ptr, output++, ptr++;
			// skip consecutive line endings
			//while (ptr != end && isEOL(*ptr))
			//	ptr++;
		}
	}
	*output = 0; // don't forget the zero terminator
	return output + 1 - outputStart; // return the size of the output
}

SODL_Loader::result SODL_Loader::loadObjectImpl(const char* buf, size_t length) {
	// 1. read object type
	// 2. instantiate object
	// 3. call mergeObject on intantiated object with the rest of the buffer
	return result { false, "not implemented", nullptr };
}

SODL_Loader::result SODL_Loader::mergeObjectImpl(ISODL_Object* object, const char* buf, size_t length) {
	return result { false, "not implemented", object };
}
