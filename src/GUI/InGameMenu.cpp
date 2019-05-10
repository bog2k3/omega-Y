#include "InGameMenu.h"

InGameMenu::InGameMenu(glm::vec2 size)
	: VerticalMenu(size)
{
	std::vector<buttonDescriptor> buttons;

	buttons.push_back({
		"Resume",
		[this](...) {
			onBack.trigger();
		}
	});

	buttons.push_back({
		"Quit",
		[this](...) {
			onQuit.trigger();
		}
	});

	setButtons(buttons);
}
