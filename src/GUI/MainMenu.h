#pragma once

#include "VerticalMenu.h"
#include <boglfw/utils/Event.h>

class MainMenu : public VerticalMenu {
public:
	MainMenu(glm::vec2 size);
	~MainMenu() override {}

	Event<void()> onSinglePlayer;
	Event<void()> onJoinMulti;
	Event<void()> onHostMulti;
	Event<void()> onExit;
private:
};
