#pragma once

#include "Session.h"
#include "GameConfig.h"

struct SessionConfig {
	Session::SessionType type;
	std::string hostAddress;
	unsigned hostPort;

	GameConfig gameConfig;
};
