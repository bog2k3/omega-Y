#pragma once

#include "VerticalMenu.h"
#include <boglfw/utils/Event.h>

#include <memory>

class LobbyMenu : public VerticalMenu {
public:
	LobbyMenu(glm::vec2 size);
	~LobbyMenu() override {}

	Event<void()> onBack;
private:
};
