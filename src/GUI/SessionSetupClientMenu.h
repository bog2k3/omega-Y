#pragma once

#include <boglfw/GUI/GuiContainerElement.h>
#include <boglfw/utils/Event.h>

#include <memory>

class Picture;

class SessionSetupClientMenu : public GuiContainerElement {
public:
	SessionSetupClientMenu(glm::vec2 viewportSize);
	~SessionSetupClientMenu() override {}

	Event<void()> onBack;

	void setRTTexture(int texId);

private:
	std::shared_ptr<Picture> pTerrainPicture_;
};
