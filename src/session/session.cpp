#include "session.h"

#include "../GUI/MainMenu.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>

Session* createLobbySession() {
	Session* s = new Session(Session::LOBBY);

	auto guiSystem = World::getGlobal<GuiSystem>();
	auto mainMenu = std::make_shared<MainMenu>(guiSystem->getViewportSize());
	guiSystem->addElement(mainMenu);

	mainMenu->onExit.add([s]() {
		s->onNewSessionRequest.trigger(Session::EXIT_GAME);
	});

	return s;
}

Session* createHostSession() {
	Session* s = new Session(Session::HOST_SETUP);
	return s;
}

Session* createJoinSession() {
	Session* s = new Session(Session::JOIN_WAIT);
	return s;
}

Session* createGameSession() {
	Session* s = new Session(Session::GAME);
	s->enableWaterRender_ = true;
	return s;
}

Session::Session(SessionType type)
	: type_(type) {
}

Session::~Session() {
	World::getGlobal<GuiSystem>()->clear();
}

void Session::update(float dt) {

}
