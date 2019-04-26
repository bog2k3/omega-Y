#pragma once

#include "../terrain/TerrainConfig.h"

#include <boglfw/GUI/GuiContainerElement.h>
#include <boglfw/utils/Event.h>

#include <memory>

class Picture;
class Label;

class SessionSetupHostMenu : public GuiContainerElement {
public:
	SessionSetupHostMenu(glm::vec2 viewportSize, TerrainConfig* pData);
	~SessionSetupHostMenu() override {}

	Event<void()> onBack;
	Event<void()> onRegenerate;
	Event<void()> onToggleWireframe;

	Event<void(float x, float y)> onTerrainStartDrag;
	Event<void()> onTerrainEndDrag;
	Event<void(float dx, float dy)> onTerrainDrag;
	Event<void(float dz)> onTerrainZoom;

	Event<void()> onParametersChanged; // triggered when terrain config parameters are changed

	void setRTTexture(int texId);
	glm::vec2 terrainPictureSize() const;

private:
	std::shared_ptr<Picture> pTerrainPicture_;
	TerrainConfig* pData_;

	void onMinElevationChanged(std::shared_ptr<Label> label, float value);
	void onMaxElevationChanged(std::shared_ptr<Label> label, float value);
};
