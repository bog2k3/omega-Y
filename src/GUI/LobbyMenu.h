#pragma once

#include <boglfw/GUI/GuiContainerElement.h>
#include <boglfw/utils/Event.h>

#include <memory>

class LobbyMenu : public GuiContainerElement {
public:
	LobbyMenu(glm::vec2 viewportSize);
	~LobbyMenu() override;

	Event<void()> onBack;
	Event<void(std::string ip)> onJoinHost;

	void addHost(std::string ip);
private:
	struct LobbyData;
	LobbyData *pData_;
};
