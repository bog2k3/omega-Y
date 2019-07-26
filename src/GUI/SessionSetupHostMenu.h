#pragma once

#include "../terrain/TerrainConfig.h"

#include <boglfw/GUI/GuiContainerElement.h>
#include <boglfw/utils/Event.h>

#include <memory>

class Label;

class SessionSetupHostMenu : public GuiContainerElement {
public:
	SessionSetupHostMenu(TerrainConfig* pData);
	~SessionSetupHostMenu() override;

	void load();

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
	struct Controls;
	Controls *pControls_;
	TerrainConfig* pData_;

	void onSeedChanged();
	void onRandSeed();
	void onTerrainParameterChanged(float* pParam, float value, std::shared_ptr<Label> *label);
	void onRandomizeAll();
};
