#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <boglfw/utils/Event.h>

class GameState {
public:
	enum class StateNames {
		MAIN_MENU,				// main menu and other settings menus before a session is established
		LOBBY,					// select a game to join from a list or by entering IP
		SESSION_SETUP_HOST,		// session setup screen while hosting
		SESSION_SETUP_CLIENT,	// session setup screen while joining
		GAMEPLAY,				// during game play
		EXIT_GAME,
	};

	~GameState();

	StateNames name() const { return name_; }

	Event<void(StateNames)> onNewStateRequest;

	static GameState* createState(StateNames name);

private:
	GameState(StateNames name);

	StateNames name_;

	static GameState* createMainMenuState();
	static GameState* createLobbyState();
	static GameState* createSessionSetupHostState();
	static GameState* createSessionSetupClientState();
	static GameState* createGameplayState();
};

#endif // GAME_STATE_H
