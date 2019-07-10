#pragma once

#include <sstream>
#include <string>

class strcat {
public:
	strcat();

	operator std::string() const {
		return sstream_.str();
	}

	template <class C>
	strcat& operator << (C const& c) {
		sstream_ << c;
		return *this;
	}

private:
	std::stringstream sstream_;
};
