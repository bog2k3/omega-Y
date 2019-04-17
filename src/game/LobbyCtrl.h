#ifndef LOBBY_CONTROLLER_H
#define LOBBY_CONTROLLER_H

#include "StateController.h"

#include <memory>

class LobbyMenu;

class LobbyCtrl : public StateController
{
public:
	LobbyCtrl(GameState &state);
	~LobbyCtrl() override;

private:
	std::shared_ptr<LobbyMenu> menu_;
};

#endif // LOBBY_CONTROLLER_H
