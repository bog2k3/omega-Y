#pragma once

#include <sstream>
#include <string>

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
