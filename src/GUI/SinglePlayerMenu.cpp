#include "SinglePlayerMenu.h"

#include <boglfw/GUI/controls/RoundButton.h>

static const float margin = 0.1f; // of screen size

SinglePlayerMenu::SinglePlayerMenu(glm::vec2 size)
	: GuiContainerElement(size * margin, size * (1 - 2*margin))
{
	setTransparentBackground(true);

	auto pStart = std::make_shared<RoundButton>(glm::vec2{50.f, 50.f}, 30.f, "Start");
	addElement(pStart);
	auto pBack = std::make_shared<RoundButton>(glm::vec2{50.f, 150.f}, 30.f, "Back");
	addElement(pBack);

	/*std::vector<buttonDescriptor> buttons;
	buttons.push_back({
		"Start",
		[this](...) {
			onStart.trigger();
		}
	});

	buttons.push_back({
		"Back",
		[this](...) {
			onBack.trigger();
		}
	});

	setButtons(buttons);*/
}
