#pragma once

#include <string>

enum class SessionType {
	HOST,
	CLIENT,
};

struct SessionConfig {
	SessionType type;
	std::string hostAddress;
	unsigned hostPort;
};
