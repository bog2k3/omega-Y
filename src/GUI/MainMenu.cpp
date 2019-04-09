#include "MainMenu.h"

MainMenu::MainMenu(glm::vec2 size)
	: VerticalMenu(size)
{
	std::vector<buttonDescriptor> buttons;
	buttons.push_back({
		"Single Player",
		[this](Button* b) {
			onSinglePlayer.trigger();
		}
	});

	buttons.push_back({
		"Host Multi Player",
		[this](Button* b) {
			onHostMulti.trigger();
		}
	});

	buttons.push_back({
		"Join Multi Player",
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
