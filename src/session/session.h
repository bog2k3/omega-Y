#ifndef SESSION_H
#define SESSION_H

#include <boglfw/utils/drawable.h>
#include <boglfw/utils/updatable.h>

#include <vector>

class Session {
public:
	enum SessionType {
		LOBBY,		// main menu before a game session is established
		HOST_SETUP,	// host game setup screen
		JOIN_WAIT,	// after joining a session that is being setup by the host
		GAME,		// during game play, after the session is started by the host
	};

	Session(SessionType type);
	~Session();

	SessionType type() const { return type_; }

	std::vector<drawable> & drawList3D() { return drawList3D_; }
	std::vector<drawable> & drawList2D() { return drawList2D_; }

	void update(float dt);

private:
	SessionType type_;
	std::vector<drawable> drawList3D_;
	std::vector<drawable> drawList2D_;
};

Session* createLobbySession();
Session* createHostSession();
Session* createJoinSession();
Session* createGameSession();

#endif // SESSION_H
