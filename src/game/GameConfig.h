#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include "../terrain/TerrainConfig.h"

enum class GameModes {
	DEATHMATCH,
	CAPTURE_THE_FORT,
};

struct GameConfig {
	GameModes gameMode;
	TerrainConfig terrainConfig;
};

#endif // GAME_CONFIG_H
