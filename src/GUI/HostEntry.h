#pragma once

#include <boglfw/GUI/GuiContainerElement.h>

class HostEntry : public GuiContainerElement {
public:
	HostEntry(glm::vec2 pos, glm::vec2 size, std::string hostName, std::string ip);
	~HostEntry() override;

private:
	std::string hostName_;
	std::string ip_;
};
