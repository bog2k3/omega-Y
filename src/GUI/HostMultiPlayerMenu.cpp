#include "HostMultiPlayerMenu.h"

#include <boglfw/GUI/controls/Button.h>
#include <boglfw/GUI/controls/Label.h>

static const float margin = 0.02f; // of screen size

HostMultiPlayerMenu::HostMultiPlayerMenu(glm::vec2 viewportSize)
	: GuiContainerElement(viewportSize * margin, viewportSize * (1 - 2*margin))
{
	glm::vec2 mySize = getSize();

	std::shared_ptr<Label> pTitle = std::make_shared<Label>(glm::vec2{mySize.x / 2, 50}, 20, "Host Game");
	addElement(pTitle);

	std::shared_ptr<Button> pBack = std::make_shared<Button>(glm::vec2{10, mySize.y - 60}, glm::vec2{200, 50}, "Back");
	pBack->onClick.add([this](auto) {
		onBack.trigger();
	});
	addElement(pBack);
}
