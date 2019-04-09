#pragma once

#include "VerticalMenu.h"
#include <boglfw/utils/Event.h>

#include <memory>

class JoinMultiPlayerMenu : public VerticalMenu {
public:
	JoinMultiPlayerMenu(glm::vec2 size);
	~JoinMultiPlayerMenu() override {}

	Event<void()> onBack;
private:
};
