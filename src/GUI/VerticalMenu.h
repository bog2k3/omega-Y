#pragma once

#include <boglfw/GUI/GuiContainerElement.h>
#include <boglfw/GUI/controls/Button.h>
#include <boglfw/GUI/GuiTheme.h>

#include <vector>
#include <utility>

class VerticalMenu : public GuiContainerElement {
public:
	VerticalMenu(glm::vec2 size);
	virtual ~VerticalMenu() override {}

protected:
	using buttonHandler = decltype(Button::onClick)::handler_type;
	struct buttonDescriptor {
		std::string name;
		buttonHandler onClick;
		bool customColor = false;
		glm::vec4 color = GuiTheme::getButtonColor();
	};
	void setButtons(std::vector<buttonDescriptor> buttons);
};
