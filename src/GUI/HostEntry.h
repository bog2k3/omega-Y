#pragma once

#include <boglfw/GUI/GuiContainerElement.h>
#include <boglfw/utils/Event.h>

class HostEntry : public GuiContainerElement {
public:
	HostEntry(std::string hostName, std::string ip);
	~HostEntry() override;

	Event<void()> onClick;

protected:
	void clicked(glm::vec2 clickPosition, MouseButtons button) override;

private:
	std::string hostName_;
	std::string ip_;
};
