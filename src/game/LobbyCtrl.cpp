#include "LobbyCtrl.h"

#include "../GUI/LobbyMenu.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>

LobbyCtrl::LobbyCtrl(GameState &s)
	: StateController(s)
{
	auto guiSystem = World::getGlobal<GuiSystem>();
	menu_ = std::make_shared<LobbyMenu>(guiSystem->getViewportSize());
	guiSystem->addElement(menu_);

	menu_->onBack.add([this]() {
		// delete session
		// ...
		onNewStateRequest.trigger(GameState::StateNames::MAIN_MENU);
	});
}

LobbyCtrl::~LobbyCtrl() {
	World::getGlobal<GuiSystem>()->removeElement(menu_);
}

