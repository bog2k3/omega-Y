#pragma once

#include <boglfw/GUI/GuiContainerElement.h>
#include <boglfw/GUI/controls/Button.h>
#include <boglfw/GUI/GuiTheme.h>

#include <vector>

class VerticalMenu : public GuiContainerElement {
public:
	VerticalMenu(glm::vec2 size);
	virtual ~VerticalMenu() override {}

protected:
	struct buttonDescriptor {
		std::string name;
		Button::buttonHandler onClick;
		decltype(Button::onClick) *forwardClick;
		bool customColor = false;
		glm::vec4 color = GuiTheme::getButtonColor();
	};
	void setButtons(std::vector<buttonDescriptor> buttons);
};
