#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include "../terrain/TerrainConfig.h"

#include <string>

enum class GameModes {
	DEATHMATCH,
	CAPTURE_THE_FORT,
};

struct GameConfig {
	GameModes gameMode;
	TerrainConfig terrainConfig;
	std::string skyBoxPath { "data/textures/sky/1" };
};

#endif // GAME_CONFIG_H
