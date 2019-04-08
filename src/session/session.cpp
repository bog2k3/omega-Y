#include "session.h"

Session* createLobbySession() {
	Session* s = new Session(Session::LOBBY);
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
	return s;
}

Session::Session(SessionType type)
	: type_(type) {
}

Session::~Session() {

}

void Session::update(float dt) {

}
