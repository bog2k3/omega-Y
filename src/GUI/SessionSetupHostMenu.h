#pragma once

#include <boglfw/GUI/GuiContainerElement.h>
#include <boglfw/utils/Event.h>

#include <memory>

class Picture;

class SessionSetupHostMenu : public GuiContainerElement {
public:
	SessionSetupHostMenu(glm::vec2 viewportSize);
	~SessionSetupHostMenu() override {}

	Event<void()> onBack;

	void setRTTexture(int texId);
	glm::vec2 terrainPictureSize() const;

private:
	std::shared_ptr<Picture> pTerrainPicture_;
};
