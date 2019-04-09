#pragma once

#include <boglfw/GUI/GuiContainerElement.h>
#include <boglfw/utils/Event.h>

class SinglePlayerMenu : public GuiContainerElement {
public:
	SinglePlayerMenu(glm::vec2 size);
	~SinglePlayerMenu() override {}

	Event<void()> onStart;
	Event<void()> onBack;
private:
};
