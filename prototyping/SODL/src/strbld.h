#pragma once

#include <sstream>
#include <string>

// String Builder class - use to quickly build an std::string in a single line of code
// by concatenating multiple streamable objects of various types
//
// example:
// std::string out = strbld() << "some text " << intVar << " other text " << whatever_other_var << " and so on";
//
class strbld {
public:
	operator std::string() const {
		return sstream_.str();
	}

	template <class C>
	strbld& operator << (C const& c) {
		sstream_ << c;
		return *this;
	}

private:
	std::stringstream sstream_;
};
