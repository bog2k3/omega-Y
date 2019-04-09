#pragma once

#include "VerticalMenu.h"
#include <boglfw/utils/Event.h>

#include <memory>

class HostMultiPlayerMenu : public VerticalMenu {
public:
	HostMultiPlayerMenu(glm::vec2 size);
	~HostMultiPlayerMenu() override {}

	// Event<void()> onSinglePlayer;
	Event<void()> onBack;
private:
};
