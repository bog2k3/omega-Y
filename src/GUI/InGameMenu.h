#pragma once

#include <boglfw/GUI/GuiContainerElement.h>
#include <boglfw/utils/Event.h>

#include <memory>

class InGameMenu : public GuiContainerElement {
public:
	InGameMenu();
	~InGameMenu() override {}

	Event<void()> onBack;
	Event<void()> onQuit;

private:
	void addButton(const char* text, Event<void()> &onClick);
};
