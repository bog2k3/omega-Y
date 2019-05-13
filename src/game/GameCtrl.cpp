#include "GameCtrl.h"

#include "../GUI/InGameMenu.h"
#include "Session.h"
#include "Game.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>

GameCtrl::GameCtrl(GameState &s)
	: StateController(s)
{
	auto guiSystem = World::getGlobal<GuiSystem>();
	menu_ = std::make_shared<InGameMenu>(guiSystem->getViewportSize());
	guiSystem->addElement(menu_);
	menu_->onBack.add(std::bind(&GameCtrl::toggleMenu, this));
	menu_->onQuit.add(std::bind(&GameCtrl::quit, this));
	menu_->hide();

	signalHandlers_[StateSignals::ESCAPE] = std::bind(&GameCtrl::toggleMenu, this);

	GameState::session()->game()->start();
}

GameCtrl::~GameCtrl() {
	World::getGlobal<GuiSystem>()->removeElement(menu_);
}

void GameCtrl::toggleMenu() {
	if (menu_->isVisible())
		menu_->hide();
	else
		menu_->show();
	World::getGlobal<GuiSystem>()->showMousePointer(menu_->isVisible());
}

void GameCtrl::quit() {
	state_.destroySession();
	onNewStateRequest.trigger(GameState::StateNames::MAIN_MENU);
}
