#include "HostMultiPlayerMenu.h"

HostMultiPlayerMenu::HostMultiPlayerMenu(glm::vec2 size)
	: VerticalMenu(size)
{
	std::vector<buttonDescriptor> buttons;

	buttons.push_back({
		"Back",
		[this](...) {
			onBack.trigger();
		}
	});

	setButtons(buttons);
}
