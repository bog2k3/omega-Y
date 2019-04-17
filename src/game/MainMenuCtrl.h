#ifndef MAINMENUCONTROLLER_H
#define MAINMENUCONTROLLER_H

#include "StateController.h"

#include <memory>

class MainMenu;

class MainMenuController : public StateController
{
public:
	MainMenuController(GameState &state);
	~MainMenuController() override;

private:
	std::shared_ptr<MainMenu> mainMenu_;
};

#endif // MAINMENUCONTROLLER_H
