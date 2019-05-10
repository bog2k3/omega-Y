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

	signalHandlers_[StateSignals::ESCAPE] = [this, guiSystem]() {
		if (menu_->isVisible())
			menu_->hide();
		else
			menu_->show();
		guiSystem->showMousePointer(menu_->isVisible());
	};

	GameState::session()->start();
}

GameCtrl::~GameCtrl() {
	World::getGlobal<GuiSystem>()->removeElement(menu_);
}
