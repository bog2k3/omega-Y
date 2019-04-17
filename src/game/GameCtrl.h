#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include "StateController.h"

#include <memory>

class InGameMenu;

class GameCtrl : public StateController
{
public:
	GameCtrl(GameState &state);
	~GameCtrl() override;

private:
	std::shared_ptr<InGameMenu> menu_;
};

#endif // GAME_CONTROLLER_H
