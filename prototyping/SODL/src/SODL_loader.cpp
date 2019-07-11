#include "SODL_loader.h"
#include "ISODL_Object.h"
#include "strcat.h"

#include <boglfw/utils/filesystem.h>
#include <boglfw/utils/assert.h>

#include <fstream>

/*
	SODL file structure
	--------------------------------------------

	the file must contain a single root node (if it contains more, the next ones after the 1st are ignored)
	each node can contain child nodes.

	comments are C-style line comments with // ...
	there are no special characters to terminate property or node definition, end-of-line is used instead
	(or {} in the case of blocks)
	whitespace doesn't have any meaning appart from separating values
	colon (:) is used to separate property name from value within blocks

	definition of a node:

	[node type] [opt: node id] [opt: node primary properties values] [opt: {]
		[opt: secondary or primary properties that were not already defined in the header]
		[opt: child nodes follow the same definition rules]
	[opt: } if the block was opened, this is mandatory]

	node type is specified as a single camelCase word (ex: container)
		For the root node, the type may be omitted, and in this case the file can not be loaded with
			loadObject because this one needs to know the node type to instantiate the correct object
		instead it can be loaded with
			mergeObject passing in a user-instantiated object that should be of the correct type

	node id must begin with a # (all ids begin with #) (ex: #rootNode)
	primary properties defined in the header are value only (ex: "someString" 13 42% enumValue)
	secondary properties are defined as camelCase identifier followed by colon (:) and value (ex: size: 12 50%)
	some secondary properties may be nodes themselves with id, primary/secondary properties and body (ex:
		layout: list #listId {
			align: right
		}
	)

	class definitions:

	class className nodeType [primaryPropValues] {
		secondaryProps: ...
		childNodes { ... }
	}

	A class is just a regular node that can be instantiated multiple times and augmented with more properties and children each time
	To instantiate a class the @ operator is used:

	@className {
		secondaryProp: 13
		childType #child1 "primaryPropVal" {
			...
		}
	}
	note that there can be no primary properties in header of a class instantiation, they must be defined in the body with "name: value" syntax

	example file : ------------------------------------

	#root {
		size: 100% 100%		// values can be absolute or relative
		padding: 50 50 50 50 // top, right, bottom, left
		layout: split vertical 10% {
			marker: #splitTop	// where to split the controls between first and second layout nodes
			first: fill
			second: fill
		}
		label #title "Host Game" 50 center	// one line child with no block
		marker #splitTop
		picture #TerrainPicture
		marker #splitPicture
		container #controls {			// container is the node type
			layout: list {
				align: right
			}
			class controlRow container {	// class definition - a class of type container with name controlRow
				height: 50					// class has properties and even child nodes that are inherited by all instantiations
				layout: list horizontal {
					align: right
					vertAlign: bottom
				}
			}
			@controlRow {						// class instantiation
				label "Seed" 18
				textField numeric $terrainSeed {	// everything that starts with $ is a data/callback binding
					width: 150
					onChange: $seedChanged
				}
				button "Random" $randSeed
			}
		}
	}
	// end of file

 */

static bool isWhitespace(const char c) {
	return c == ' ' || c == '\t' || c == '\r';
}

static bool isEOL(const char c) {
	return c == '\n';
}

class SODL_Loader::ParseStream {
public:
	ParseStream(const char* buf, size_t length)
		: bufStart_(buf)
		, bufCrt_(buf)
		, bufEnd_(buf + length)
	{}

	~ParseStream() {}

	// returns the next usable char in the stream, skipping whitespace and line ends
	char nextChar() {
		const char* ptr = bufCrt_;
		while (ptr < bufEnd_ && (isWhitespace(*ptr) || isEOL(*ptr)))
			ptr++;
		if (ptr == bufEnd_)
			return 0;
		else
			return *ptr;
	}

	void skipChar(char c) {
		skipWhitespace(true);
		assertDbg(bufCrt_ < bufEnd_ && *bufCrt_ == c);
		bufCrt_++;
		skipWhitespace(false);
	}
	
	void skipEOL() {
		assertDbg(eol());
		skipWhitespace(true);
	}

	bool eol() {
		return bufCrt_ < bufEnd_ && isEOL(*bufCrt_);
	}

	bool eof() {
		return bufCrt_ >= bufEnd_;
	}

	SODL_result readValue(SODL_Value::Type type, SODL_Value &out_val) {
		return SODL_result::error("not implemented");
	}
	
	bool nextIsWhitespace() {
		return !eof() && isWhitespace(*bufCrt_);
	}

	SODL_result readIdentifier(std::string &out_str) {
		skipWhitespace(false);
		const char* idStart = bufCrt_;
		// first char must be treated special
		if (eof())
			return SODL_result::error("Reached end of file while expecting an identifier.");
		if (!isValidFirstChar(*bufCrt_))
			return SODL_result::error("Identifier starts with invalid character.");
		bufCrt_++;
		while (!eof() && isValidIdentChar(*bufCrt_))
			bufCrt_++;
		out_str = std::string(idStart, bufCrt_);
		skipWhitespace(false);
		return SODL_result::OK();
	}

private:
	const char* bufStart_;
	const char* bufCrt_;
	const char* bufEnd_;

	void skipWhitespace(bool skipLineEnd) {
		while (bufCrt_ < bufEnd_ && (isWhitespace(*bufCrt_) || (skipLineEnd && isEOL(*bufCrt_))))
			bufCrt_++;
	}
	
	// returns true if the char is valid as a first character of an identifier
	bool isValidFirstChar(char c) {
		return (c >= 'a' && c <= 'z')
			|| (c >= 'A' && c <= 'Z')
			|| c == '_';
	}
	
	// returns true if the char is a valid char in the middle of an identifier
	bool isValidIdentChar(char c) {
		return isValidFirstChar(c)
			|| (c >= '0' && c <= '9');
	}
};

SODL_Loader::~SODL_Loader() {
	for (auto &pair : mapActionBindings_)
		if (pair.second)
			delete pair.second;
	mapActionBindings_.clear();
}

void SODL_Loader::addDataBinding(const char* name, SODL_Value::Type type, void* valuePtr) {
	// TOOD
}

// loads a SODL file and returns a new ISODL_Object (actual type depending on the root node's type in the file)
SODL_result SODL_Loader::loadObject(const char* filename, ISODL_Object* &out_pObj) {
	auto text = readFile(filename);
	if (!text.first || !text.second) {
		return SODL_result::error("Unable to open SODL file");
	}
	ParseStream stream(text.first, text.second);
	return loadObjectImpl(stream, out_pObj);
}

// merges a SODL file with an existing ISODL_Object provided by user; The root node in the file mustn't specify a type.
SODL_result SODL_Loader::mergeObject(ISODL_Object &object, const char* filename) {
	auto text = readFile(filename);
	if (!text.first || !text.second) {
		return SODL_result::error("Unable to open SODL file");
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

SODL_result SODL_Loader::loadObjectImpl(SODL_Loader::ParseStream &stream, ISODL_Object* &out_pObj) {
	// 1. read object type
	// 2. instantiate object
	// 3. call mergeObject on intantiated object with the rest of the buffer
	SODL_result res;
	do {
		std::string objType;
		res = readObjectType(stream, objType);
		if (!res)
			break;
		res = instantiateObject(objType, out_pObj);
		if (!res)
			break;
		assertDbg(out_pObj);
		res = mergeObjectImpl(*out_pObj, stream);
	} while (0);
	return res;
}

SODL_result SODL_Loader::mergeObjectImpl(ISODL_Object &object, SODL_Loader::ParseStream &stream) {
	SODL_result res;
	do {
		res = readPrimaryProps(object, stream);
		if (!res)
			break;
		if (stream.nextChar() == '{')
			res = readObjectBlock(object, stream);
	} while (0);
	return res;
}

SODL_result SODL_Loader::readObjectType(SODL_Loader::ParseStream &stream, std::string &out_type) {
	return stream.readIdentifier(out_type);
}

SODL_result SODL_Loader::instantiateObject(std::string const& objType, ISODL_Object* &out_pObj) {
	SODL_result res = factory_.constructObject(objType, out_pObj);
	if (!res)
		res.errorMessage = "Failed to construct object of type [" + objType + "]; Reason: \"" + res.errorMessage + "\"";
	return res;
}

SODL_result SODL_Loader::assignPropertyValue(ISODL_Object &object, SODL_Property_Descriptor const& desc, 
	SODL_Value& val, unsigned primaryPropIdx, const char* propName) {
	SODL_result res;
	if (val.isBinding) {
		if (desc.type == SODL_Value::Type::Callback) {
			// retrieve the registered action callback for this binding and set it onto the object
			auto it = mapActionBindings_.find(val.bindingName);
			if (it == mapActionBindings_.end())
				return SODL_result::error("Action $" + val.bindingName + " was not defined.");
			auto &actionDesc = *it->second;
			res = checkCallbackArgumentsMatch(actionDesc.argTypes_, desc.callbackArgTypes);
			if (!res)
				return res;
			if (desc.callbackPtr == nullptr)
				return SODL_result::error(strcat() << "Callback property " << propIdx << " does not supply a function pointer");
			actionDesc.pBindingWrapper_->setObjectCallbackBinding(desc.callbackPtr);
		} else {
			// retrieve the registered data binding and set its value onto the object
			res = resolveDataBinding(val, desc.type);
			if (!res)
				return res;
			if (propName == nullptr)
				res = object.setPrimaryProperty(propIdx, val);
			else
				res = object.setPropertyValue(propName, val);
		}
	} else if (propName == nullptr)
		res = object.setPrimaryProperty(propIdx, val);
	else
		res = object.setPropertyValue(propName, val);
	return res;
}

SODL_result SODL_Loader::readPrimaryProps(ISODL_Object &object, SODL_Loader::ParseStream &stream) {
	SODL_result res;
	if (stream.nextChar() == '#') {
		stream.skipChar('#');
		if (stream.nextIsWhitespace()) {
			return SODL_result::error("Expected identifier after #, found white space");
		}
		std::string idStr;
		res = stream.readIdentifier(idStr);
		if (!res)
			return res;
		object.setId(idStr);
	}
	unsigned propIdx = 0;
	while (!stream.eof() && !stream.eol() && stream.nextChar() != '{') {
		SODL_Property_Descriptor desc = object.describePrimaryProperty(propIdx);
		assertDbg(!desc.isObject);
		SODL_Value val;
		res = stream.readValue(desc.type, val);
		if (!res)
			return res;
		res = assignPropertyValue(object, val, propIdx, nullptr);
		if (!res)
			return res;
		propIdx++;
	}
	if (stream.eol()) {
		stream.skipEOL();
	}
	return SODL_result::OK();
}

SODL_result SODL_Loader::resolveDataBinding(SODL_Value &inOutVal, SODL_Value::Type expectedType) {
	// 1. look up the data bindings map using inOutVal.bindingName as key
	// 2. check its type against expectedType
	// 3. update inOutVal's actual value to the retrieved data, and type to expectedType
}

SODL_result SODL_Loader::checkCallbackArgumentsMatch(std::vector<SODL_Value::Type> argTypes, std::vector<SODL_Value::Type> expectedTypes) {
	if (argTypes.size() != expectedTypes.size())
		return SODL_result::error("Action binding mismatch: wrong number of arguments");
	for (unsigned i=0; i<argTypes.size(); i++) {
		if (argTypes[i] != expectedTypes[i])
			return SODL_result::error(strcat() << "Action binding mismatch: argument " << i+1 << " type mismatch");
	}
	return SODL_result::OK();
}

SODL_result SODL_Loader::readObjectBlock(ISODL_Object &object, SODL_Loader::ParseStream &stream) {
	assertDbg(stream.nextChar() == '{');
	stream.skipChar('{');
	while (!stream.eof() && stream.nextChar() != '}') {
		if (stream.eol()) {
			stream.skipEOL();
			continue;
		}
		bool classInstance = false;
		if (stream.nextChar() == '@') {
			classInstance = true;
			stream.skipChar('@');
			if (stream.nextIsWhitespace()) {
				return SODL_result::error("Expected identifier after @, found white space");
			}
		}
		std::string ident;
		SODL_result res = stream.readIdentifier(ident);
		if (!res)
			return res;
		if (classInstance) {
			ISODL_Object *pInstanceObj = nullptr;
			res = object.instantiateClass(ident, pInstanceObj);
			if (!res)
				return res;
			res = mergeObjectImpl(*pInstanceObj, stream);
			if (!res)
				return res;
			res = object.addChildObject(pInstanceObj);
			if (!res)
				return res;
		} else if (ident == "class") {
			res = readClass(object, stream);
			if (!res)
				return res;
		} else if (stream.nextChar() == ':') {
			// this is a property
			stream.skipChar(':');
			SODL_Property_Descriptor pdesc = object.describeProperty(ident);
			SODL_PropValue pvalue;
			if (pdesc.isObject) {
				res = factory_.constructObject(ident, pvalue.pObject);
				if (!res)
					return res;
				res = mergeObjectImpl(*pvalue.pObject, stream);
				if (!res) {
					delete pvalue.pObject;
					return res;
				}
				res = object.setPropertyObject(ident, pvalue.pObject);
			} else {
				res = stream.readValue(pdesc.type, pvalue.simpleValue);
				if (!res)
					return res;
				res = assignPropertyValue(object, pdesc, pvalue.simpleValue, 0, ident);
			}
			if (pvalue.pObject)
				delete pvalue.pObject;
			if (!res)
				return res;
		} else {
			// this must be a child object
			ISODL_Object *pObj = nullptr;
			res = loadObjectImpl(stream, pObj);
			if (!res)
				return res;
			res = object.addChildObject(pObj);
			if (!res)
				return res;
		}
	}
	if (stream.eof())
		return SODL_result::error("End of file while expecting '}'");
	stream.skipChar('}');
	return SODL_result::OK();
}

SODL_result SODL_Loader::readClass(ISODL_Object &object, SODL_Loader::ParseStream &stream) {

}