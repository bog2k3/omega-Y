#include "Session.h"
#include "Game.h"
#include "../net/NetHost.h"
#include "../net/NetClient.h"

Session::Session(SessionConfig cfg)
	: type_(cfg.type) {
	if (type_ == SessionType::HOST)
		pNetHost_ = new NetHost(cfg.hostPort);
	else
		pNetClient_ = new NetClient(cfg.hostAddress, cfg.hostPort);
}

Session::~Session() {
	//close network connections
	if (pNetHost_)
		delete pNetHost_, pNetHost_ = nullptr;
	if (pNetClient_)
		delete pNetClient_, pNetClient_ = nullptr;
	// destroy the game
	if (game_)
		destroyGame();
}

void Session::initializeGame() {
	assertDbg(game_ == nullptr);
	game_ = new Game(gameCfg_);
	game_->onStart.forward(onGameStart);
	game_->onEnd.forward(onGameEnd);
}

void Session::destroyGame() {
	assertDbg(game_ && !game_->isStarted());
	delete game_, game_ = nullptr;
}

NetAdapter* Session::netAdapter() const {
	NetAdapter* pAdapter = type_ == SessionType::HOST ? (NetAdapter*)pNetHost_ : (NetAdapter*)pNetClient_;
	assertDbg(pAdapter);
	return pAdapter;
}
