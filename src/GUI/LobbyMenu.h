#pragma once

#include <boglfw/GUI/GuiContainerElement.h>
#include <boglfw/utils/Event.h>

#include <memory>

class LobbyMenu : public GuiContainerElement {
public:
	LobbyMenu(glm::vec2 viewportSize);
	~LobbyMenu() override;

	Event<void()> onBack;

	void addHost(std::string ip);
private:
	struct LobbyData;
	LobbyData *pData_;
};
