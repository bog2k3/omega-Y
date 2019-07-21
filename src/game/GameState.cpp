#include "GameState.h"
#include "LoadingCtrl.h"
#include "MainMenuCtrl.h"
#include "LobbyCtrl.h"
#include "SessionSetupHostCtrl.h"
#include "SessionSetupClientCtrl.h"
#include "GameCtrl.h"
#include "SessionConfig.h"

GameState::initSessionFunction GameState::initSessionCallback;
GameState::destroySessionFunction GameState::destroySessionCallback;
std::shared_ptr<Session> GameState::sessionPtr;

GameState* GameState::createInitialLoadingState() {
	GameState* s = new GameState(StateNames::INITIAL_LOADING);
	s->pController_ = new LoadingCtrl(*s, LoadingCtrl::INITIAL);

	return s;
}

GameState* GameState::createMainMenuState() {
	GameState* s = new GameState(StateNames::MAIN_MENU);
	s->pController_ = new MainMenuController(*s);

	return s;
}

GameState* GameState::createSessionSetupHostState() {
	GameState* s = new GameState(StateNames::SESSION_SETUP_HOST);
	s->pController_ = new SessionSetupHostCtrl(*s);

	// create the session here
	// ...

	return s;
}

GameState* GameState::createSessionSetupClientState() {
	GameState* s = new GameState(StateNames::SESSION_SETUP_CLIENT);
	s->pController_ = new SessionSetupClientCtrl(*s);

	// create the session here and populate it with data received from network
	// ...

	return s;
}

GameState* GameState::createLobbyState() {
	GameState* s = new GameState(StateNames::LOBBY);
	s->pController_ = new LobbyCtrl(*s);

	return s;
}

GameState* GameState::createGameplayState() {
	GameState* s = new GameState(StateNames::GAMEPLAY);
	s->pController_ = new GameCtrl(*s);
	return s;
}

GameState* GameState::createSessionLoadingState() {
	GameState* s = new GameState(StateNames::SESSION_LOADING);
	s->pController_ = new LoadingCtrl(*s, LoadingCtrl::SESSION_START);
	return s;
}

GameState::GameState(StateNames name)
	: name_(name) {
}

GameState::~GameState() {
	if (pController_)
		delete pController_;
}

GameState* GameState::createState(StateNames name) {
	switch (name) {
	case StateNames::INITIAL_LOADING:
		return createInitialLoadingState();
	case StateNames::MAIN_MENU:
		return createMainMenuState();
	case StateNames::LOBBY:
		return createLobbyState();
	case StateNames::SESSION_SETUP_HOST:
		return createSessionSetupHostState();
	case StateNames::SESSION_SETUP_CLIENT:
		return createSessionSetupClientState();
	case StateNames::SESSION_LOADING:
		return createSessionLoadingState();
	case StateNames::GAMEPLAY:
		return createGameplayState();
	default:
		assertDbg(false && "invalid state name");
		return nullptr;
	}
}

void GameState::initSession(SessionConfig cfg) {
	assertDbg(initSessionCallback && "No initSessionCallback provided!");
	sessionPtr = initSessionCallback(cfg);
}

void GameState::destroySession() {
	assertDbg(destroySessionCallback && "No destroySessionCallback provided!");
	sessionPtr.reset();
	destroySessionCallback();
}
