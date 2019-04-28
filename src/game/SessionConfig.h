#pragma once

#include "Session.h"

struct SessionConfig {
	Session::SessionType type;
	std::string hostAddress;
	unsigned hostPort;
};
