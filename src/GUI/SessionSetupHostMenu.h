#pragma once

#include "../terrain/TerrainConfig.h"

#include <boglfw/GUI/GuiContainerElement.h>
#include <boglfw/utils/Event.h>

#include <memory>

class Picture;
class Label;
class TextField;
class Slider;

class SessionSetupHostMenu : public GuiContainerElement {
public:
	SessionSetupHostMenu(TerrainConfig* pData);
	~SessionSetupHostMenu() override {}

	Event<void()> onBack;
	Event<void()> onStart;

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

	void onSeedChanged(std::shared_ptr<TextField> pSeedField);
	void onRandSeed(std::shared_ptr<TextField> pSeedField);
	void onTerrainParameterChanged(float* pParam, float value, std::shared_ptr<Label> label);
	void onRandomizeAll(std::shared_ptr<TextField> pSeedField,
		std::shared_ptr<Slider> pMinElevSlider, std::shared_ptr<Slider> pMaxElevSlider,
		std::shared_ptr<Slider> pRoughnessSlider);
};
