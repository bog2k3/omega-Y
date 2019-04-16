#pragma once

#include <boglfw/GUI/GuiContainerElement.h>
#include <boglfw/utils/Event.h>

#include <memory>

class Picture;

class HostMultiPlayerMenu : public GuiContainerElement {
public:
	HostMultiPlayerMenu(glm::vec2 viewportSize);
	~HostMultiPlayerMenu() override {}

	Event<void()> onBack;

	void setRTTexture(int texId);

private:
	std::shared_ptr<Picture> pTerrainPicture_;
};
