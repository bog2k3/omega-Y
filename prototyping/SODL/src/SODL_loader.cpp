#include "SODL_loader.h"
#include "ISODL_Object.h"

#include <boglfw/utils/filesystem.h>
#include <boglfw/utils/assert.h>

#include <fstream>

class SODL_Loader::ParseStream {
public:
	ParseStream(const char* buf, size_t length)
		: bufStart_(buf)
		, bufCrt_(buf)
		, length_(length)
	{}

	~ParseStream() {}

private:
	const char* bufStart_;
	const char* bufCrt_;
	const size_t length_;
};

// loads a SODL file and returns a new ISODL_Object (actual type depending on the root node's type in the file)
SODL_Loader::result SODL_Loader::loadObject(const char* filename) {
	auto text = readFile(filename);
	if (!text.first || !text.second) {
		return result { false, "Unable to open SODL file", nullptr };
	}
	ParseStream stream(text.first, text.second);
	return loadObjectImpl(stream);
}

// merges a SODL file with an existing ISODL_Object provided by user; The root node in the file mustn't specify a type.
SODL_Loader::result SODL_Loader::mergeObject(ISODL_Object* object, const char* filename) {
	auto text = readFile(filename);
	if (!text.first || !text.second) {
		return result { false, "Unable to open SODL file", nullptr };
	}
	ParseStream stream(text.first, text.second);
	return mergeObjectImpl(object, stream);
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
		if (ptr != end && isEOL(*ptr)) {
			if (commentBegining != lineStart) {
				// if there was something on the line before the comment, we also write the EOL char
				*output = *ptr, output++, ptr++;
			} else {
				ptr++;
			}
		}
	}
	*output = 0; // don't forget the zero terminator
	return output + 1 - outputStart; // return the size of the output
}

SODL_Loader::result SODL_Loader::loadObjectImpl(SODL_Loader::ParseStream &stream) {
	// 1. read object type
	// 2. instantiate object
	// 3. call mergeObject on intantiated object with the rest of the buffer
	result res;
	do {
		std::string objType;
		res = readObjectType(stream, objType);
		if (!res)
			break;
		res = instantiateObject(objType);
		if (!res)
			break;
		res = mergeObjectImpl(res.rootObject, stream);
	} while (0);
	return res;
}

SODL_Loader::result SODL_Loader::mergeObjectImpl(ISODL_Object* object, SODL_Loader::ParseStream &stream) {
	result res;
	do {
		res = readPrimaryProps(object, stream);
		if (!res)
			break;
		if (stream.nextChar() == '{')
			res = readObjectBlock(object, stream);
	} while (0);
	res.rootObject = object;
	return res;
}

SODL_Loader::result SODL_Loader::readObjectType(SODL_Loader::ParseStream &stream, std::string &out_type) {

}

SODL_Loader::result SODL_Loader::instantiateObject(std::string const& objType) {

}

SODL_Loader::result SODL_Loader::readPrimaryProps(ISODL_Object* object, SODL_Loader::ParseStream &stream) {

}

SODL_Loader::result SODL_Loader::readObjectBlock(ISODL_Object* object, SODL_Loader::ParseStream &stream) {

}
