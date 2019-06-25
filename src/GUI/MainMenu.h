#pragma once

#include <boglfw/GUI/GuiContainerElement.h>
#include <boglfw/utils/Event.h>

class MainMenu : public GuiContainerElement {
public:
	MainMenu();
	~MainMenu() override {}

	Event<void()> onJoinMulti;
	Event<void()> onHostMulti;
	Event<void()> onExit;

private:
	void addButton(const char* text, Event<void()> &onClick);
};
