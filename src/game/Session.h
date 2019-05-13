#ifndef SESSION_H
#define SESSION_H

#include "SessionConfig.h"
#include "GameConfig.h"
#include "../net/NetAdapter.h"

#include <boglfw/utils/Event.h>

#include <memory>

class Game;
class NetHost;
class NetClient;

class Session {
public:
	Session(SessionConfig cfg);
	~Session();

	void initializeGame();
	void destroyGame();

	SessionType type() const { return type_; }
	GameConfig& gameConfig() { return gameCfg_; }
	NetAdapter* netAdapter() const;

	Game* game() const { return game_; }

	Event<void()> onGameStart;
	Event<void()> onGameEnd;

private:
	SessionType type_;
	GameConfig gameCfg_;
	Game* game_ = nullptr;
	NetHost *pNetHost_ = nullptr;
	NetClient *pNetClient_ = nullptr;
};

#endif // SESSION_H
