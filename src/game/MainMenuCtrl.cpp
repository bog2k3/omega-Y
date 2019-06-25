#include "MainMenuCtrl.h"

#include "../GUI/MainMenu.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>

MainMenuController::MainMenuController(GameState &s)
	: StateController(s)
{
	mainMenu_ = std::make_shared<MainMenu>();
	World::getGlobal<GuiSystem>()->addElement(mainMenu_);

	mainMenu_->onHostMulti.add([this]() {
		onNewStateRequest.trigger(GameState::StateNames::SESSION_SETUP_HOST);
	});
	mainMenu_->onJoinMulti.add([this]() {
		onNewStateRequest.trigger(GameState::StateNames::LOBBY);
	});
	mainMenu_->onExit.add([this]() {
		onNewStateRequest.trigger(GameState::StateNames::EXIT_GAME);
	});
}

MainMenuController::~MainMenuController() {
	World::getGlobal<GuiSystem>()->removeElement(mainMenu_);
}
