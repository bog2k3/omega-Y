#include "VerticalMenu.h"

static const float margin = 0.1f; // of screen size

VerticalMenu::VerticalMenu(glm::vec2 screenSize)
	: GuiContainerElement(screenSize * margin, screenSize * (1 - 2*margin))
{
}

void VerticalMenu::setButtons(std::vector<buttonDescriptor> buttons) {
	constexpr float buttonAspectRatio = 5.f;
	constexpr int maxButtonWidth = 400;
	constexpr float relativeButtonWidthPortrait = 0.8f; // of menu width
	constexpr float relativeButtonWidthLandscape = 0.25f; // of menu width
	const float relativeButtonWidth = getSize().x > getSize().y ? relativeButtonWidthLandscape : relativeButtonWidthPortrait;
	constexpr float buttonVerticalSpacing = 0.25f; // of button height
	const float verticalExtent = (buttons.size() + 0.5); // of button height + button vertical spacing
	constexpr float maxVerticalCoverage = 0.8; // of menu height to leave some roome above and below

	int proportionalButtonWidth = getSize().x * relativeButtonWidth;
	int actualButtonWidth = std::min(maxButtonWidth, proportionalButtonWidth);
	glm::vec2 buttonSize { actualButtonWidth, actualButtonWidth / buttonAspectRatio };
	int buttonYOffs = buttonSize.y * (1 + buttonVerticalSpacing);
	float actualVerticalCoverage = buttonYOffs * verticalExtent;
	float verticalScaling = 1.f;
	if (actualVerticalCoverage > maxVerticalCoverage * getSize().y) {
		verticalScaling = maxVerticalCoverage * getSize().y / actualVerticalCoverage;
		actualVerticalCoverage *= verticalScaling;
	}
	buttonSize *= verticalScaling;
	buttonYOffs *= verticalScaling;
	int buttonX = (getSize().x - buttonSize.x) / 2;
	int buttonY = (getSize().y - actualVerticalCoverage) / 2;

	for (auto &b : buttons) {
		auto pB = std::make_shared<Button>(glm::vec2{buttonX, buttonY}, buttonSize, b.name);
		pB->onClick.add(b.onClick);
		addElement(pB);
		if (b.customColor) {
			//pB->setColor(b.color);
		}
		buttonY += buttonYOffs;
	}
}
