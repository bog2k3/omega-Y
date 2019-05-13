#pragma once

#include <string>

enum class SessionType {
	HOST,
	CLIENT,
};

struct SessionConfig {
	SessionType type;
	std::string hostAddress;
	unsigned hostPort = 17342; // just some random number we chose
};
