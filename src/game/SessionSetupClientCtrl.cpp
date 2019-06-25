#include "SessionSetupClientCtrl.h"

#include "Session.h"
#include "SessionConfig.h"

#include "../GUI/SessionSetupClientMenu.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>

SessionSetupClientCtrl::SessionSetupClientCtrl(GameState &s)
	: StateController(s)
{
	SessionConfig sessionCfg;
	sessionCfg.type = SessionType::CLIENT;
	sessionCfg.hostAddress = "localhost";
	state_.initSession(sessionCfg);

	menu_ = std::make_shared<SessionSetupClientMenu>();
	World::getGlobal<GuiSystem>()->addElement(menu_);

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
