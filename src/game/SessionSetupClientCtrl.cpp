#include "SessionSetupClientCtrl.h"

#include "../GUI/SessionSetupClientMenu.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>

SessionSetupClientCtrl::SessionSetupClientCtrl(GameState &s)
	: StateController(s)
{
	auto guiSystem = World::getGlobal<GuiSystem>();
	menu_ = std::make_shared<SessionSetupClientMenu>(guiSystem->getViewportSize());
	guiSystem->addElement(menu_);

	menu_->onBack.add([this]() {
		// delete session
		// ...
		onNewStateRequest.trigger(GameState::StateNames::MAIN_MENU);
	});
}

SessionSetupClientCtrl::~SessionSetupClientCtrl() {
	World::getGlobal<GuiSystem>()->removeElement(menu_);
}

void SessionSetupClientCtrl::update(float dt) {
	// prepare terrain picture
}
