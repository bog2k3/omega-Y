#include "GameCtrl.h"

#include "../GUI/InGameMenu.h"
#include "Session.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>

GameCtrl::GameCtrl(GameState &s)
	: StateController(s)
{
	auto guiSystem = World::getGlobal<GuiSystem>();
	menu_ = std::make_shared<InGameMenu>(guiSystem->getViewportSize());
	guiSystem->addElement(menu_);
	menu_->hide();

	GameState::session()->start();
}

GameCtrl::~GameCtrl() {
	World::getGlobal<GuiSystem>()->removeElement(menu_);
}
