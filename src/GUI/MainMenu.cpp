#include "MainMenu.h"

MainMenu::MainMenu(glm::vec2 size)
	: VerticalMenu(size)
{
	std::vector<buttonDescriptor> buttons;

	buttons.push_back({
		"Host Game",
		[this](Button* b) {
			onHostMulti.trigger();
		}
	});

	buttons.push_back({
		"Join Game",
		[this](Button* b) {
			onJoinMulti.trigger();
		}
	});

	buttons.push_back({
		"Exit",
		[this](Button* b) {
			onExit.trigger();
		}
	});


	setButtons(buttons);
}
