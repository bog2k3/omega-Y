#include "SessionSetupHostCtrl.h"

#include "../GUI/SessionSetupHostMenu.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>

SessionSetupHostCtrl::SessionSetupHostCtrl(GameState &s)
	: StateController(s)
{
	auto guiSystem = World::getGlobal<GuiSystem>();
	menu_ = std::make_shared<SessionSetupHostMenu>(guiSystem->getViewportSize());
	guiSystem->addElement(menu_);

	menu_->onBack.add([this]() {
		// delete session
		// ...
		onNewStateRequest.trigger(GameState::StateNames::MAIN_MENU);
	});
}

SessionSetupHostCtrl::~SessionSetupHostCtrl() {
	World::getGlobal<GuiSystem>()->removeElement(menu_);
}

void SessionSetupHostCtrl::draw(RenderContext const& ctx) {
	// prepare terrain picture
}
