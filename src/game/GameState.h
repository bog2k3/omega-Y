#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <boglfw/utils/assert.h>

#include <memory>

class StateController;
class Session;
struct SessionConfig;

class GameState {
public:
	enum class StateNames {
		INITIAL_LOADING,		// initial loading screen, before menu
		MAIN_MENU,				// main menu and other settings menus before a session is established
		LOBBY,					// select a game to join from a list or by entering IP
		SESSION_SETUP_HOST,		// session setup screen while hosting
		SESSION_SETUP_CLIENT,	// session setup screen while joining
		SESSION_LOADING,		// loading screen before gameplay
		GAMEPLAY,				// during game play
		EXIT_GAME,
	};

	~GameState();

	StateNames name() const { return name_; }
	StateController& controller() const { return *pController_; }
	static std::shared_ptr<Session> session() { assertDbg(sessionPtr && "session must be initialized first"); return sessionPtr; }

	static GameState* createState(StateNames name);

	static std::shared_ptr<Session> (*initSessionCallback)(SessionConfig cfg);
	static void (*destroySessionCallback)();

	void initSession(SessionConfig cfg);
	void destroySession();

private:
	GameState(StateNames name);

	const StateNames name_;
	StateController *pController_ = nullptr;

	static std::shared_ptr<Session> sessionPtr;

	static GameState* createInitialLoadingState();
	static GameState* createMainMenuState();
	static GameState* createLobbyState();
	static GameState* createSessionSetupHostState();
	static GameState* createSessionSetupClientState();
	static GameState* createSessionLoadingState();
	static GameState* createGameplayState();
};

#endif // GAME_STATE_H
