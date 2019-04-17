#include "GameState.h"

#include "../GUI/MainMenu.h"
#include "../GUI/HostMultiPlayerMenu.h"
#include "../GUI/JoinMultiPlayerMenu.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>

GameState::GameState(StateNames name)
	: name_(name) {
}

GameState::~GameState() {
	World::getGlobal<GuiSystem>()->clear();
}

GameState* GameState::createState(StateNames name) {
	switch (name) {
	case StateNames::MAIN_MENU:
		return createMainMenuState();
	case StateNames::LOBBY:
		return createLobbyState();
	case StateNames::SESSION_SETUP_HOST:
		return createSessionSetupHostState();
	case StateNames::SESSION_SETUP_CLIENT:
		return createSessionSetupClientState();
	case StateNames::GAMEPLAY:
		return createGameplayState();
	default:
		assertDbg(false && "invalid state name");
		break;
	}
}

GameState* GameState::createMainMenuState() {
	GameState* s = new GameState(StateNames::MAIN_MENU);

	auto guiSystem = World::getGlobal<GuiSystem>();
	auto mainMenu = std::make_shared<MainMenu>(guiSystem->getViewportSize());
	guiSystem->addElement(mainMenu);

	mainMenu->onHostMulti.add([s]() {
		s->onNewStateRequest.trigger(StateNames::SESSION_SETUP_HOST);
	});
	mainMenu->onJoinMulti.add([s]() {
		s->onNewStateRequest.trigger(StateNames::LOBBY);
	});
	mainMenu->onExit.add([s]() {
		s->onNewStateRequest.trigger(StateNames::EXIT_GAME);
	});

	return s;
}

GameState* GameState::createSessionSetupHostState() {
	GameState* s = new GameState(StateNames::SESSION_SETUP_HOST);

	auto guiSystem = World::getGlobal<GuiSystem>();
	auto HostMenu = std::make_shared<HostMultiPlayerMenu>(guiSystem->getViewportSize());
	guiSystem->addElement(HostMenu);

	HostMenu->onBack.add([s]() {
		// delete session
		// ...
		s->onNewStateRequest.trigger(StateNames::MAIN_MENU);
	});

	// create the session here
	// ...

	return s;
}

GameState* GameState::createLobbyState() {
	GameState* s = new GameState(StateNames::LOBBY);

	auto guiSystem = World::getGlobal<GuiSystem>();
	auto JoinMenu = std::make_shared<JoinMultiPlayerMenu>(guiSystem->getViewportSize());
	guiSystem->addElement(JoinMenu);

	JoinMenu->onBack.add([s]() {
		s->onNewStateRequest.trigger(StateNames::MAIN_MENU);
	});

	return s;
}

GameState* GameState::createSessionSetupClientState() {
	GameState* s = new GameState(StateNames::SESSION_SETUP_CLIENT);

	// create the session here and populate it with data received from network
	// ...

	return s;
}

GameState* GameState::createGameplayState() {
	GameState* s = new GameState(StateNames::GAMEPLAY);
	return s;
}

/*void GameState::update(float dt) {

}*/
