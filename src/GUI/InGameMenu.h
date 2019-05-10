#pragma once

#include "VerticalMenu.h"
#include <boglfw/utils/Event.h>

#include <memory>

class InGameMenu : public VerticalMenu {
public:
	InGameMenu(glm::vec2 size);
	~InGameMenu() override {}

	Event<void()> onBack;
	Event<void()> onQuit;
private:
};
