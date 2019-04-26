#include "MainMenu.h"

MainMenu::MainMenu(glm::vec2 size)
	: VerticalMenu(size)
{
	std::vector<buttonDescriptor> buttons;

	buttons.push_back({
		"Host Game",
		nullptr,
		&onHostMulti
	});

	buttons.push_back({
		"Join Game",
		nullptr,
		&onJoinMulti
	});

	buttons.push_back({
		"Exit",
		nullptr,
		&onExit
	});


	setButtons(buttons);
}
