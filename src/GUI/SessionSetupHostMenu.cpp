#include "SessionSetupHostMenu.h"

#include <boglfw/GUI/controls/Button.h>
#include <boglfw/GUI/controls/Label.h>
#include <boglfw/GUI/controls/Picture.h>
#include <boglfw/GUI/controls/Slider.h>

#include <sstream>
#include <iomanip>

static const float margin = 0.02f; // of screen size

SessionSetupHostMenu::SessionSetupHostMenu(glm::vec2 viewportSize, TerrainConfig* pData)
	: GuiContainerElement(viewportSize * margin, viewportSize * (1 - 2*margin))
	, pData_(pData)
{
	glm::vec2 mySize = getSize();

	std::shared_ptr<Label> pTitle = std::make_shared<Label>(glm::vec2{mySize.x / 2 - 125, 85}, 50, "Host Game");
	addElement(pTitle);

	std::shared_ptr<Button> pBack = std::make_shared<Button>(glm::vec2{10, mySize.y - 60}, glm::vec2{200, 50}, "Back");
	pBack->onClick.forward(onBack);
	addElement(pBack);

	std::shared_ptr<Button> pRegenerate = std::make_shared<Button>(glm::vec2{25, 620}, glm::vec2{160, 30}, "New terrain");
	pRegenerate->onClick.forward(onRegenerate);
	addElement(pRegenerate);

	std::shared_ptr<Button> pWireframe = std::make_shared<Button>(glm::vec2{205, 620}, glm::vec2{160, 30}, "Toggle Wireframe");
	pWireframe->onClick.forward(onToggleWireframe);
	addElement(pWireframe);

	std::shared_ptr<Label> pMinElevLabel = std::make_shared<Label>(glm::vec2{1120, 130}, 18, "");
	addElement(pMinElevLabel);

	std::shared_ptr<Slider> pMinElevSlider = std::make_shared<Slider>(glm::vec2{850, 100}, 250);
	pMinElevSlider->setLabel("Min Elevation");
	pMinElevSlider->setRange(-20, -1, 0.5f);
	pMinElevSlider->setDisplayStyle(5, 1, 0);
	pMinElevSlider->onValueChanged.add(std::bind(&SessionSetupHostMenu::onMinElevationChanged, this, pMinElevLabel, std::placeholders::_1));
	pMinElevSlider->setValue(pData_->minElevation);
	addElement(pMinElevSlider);

	std::shared_ptr<Label> pMaxElevLabel = std::make_shared<Label>(glm::vec2{1120, 180}, 18, "");
	addElement(pMaxElevLabel);

	std::shared_ptr<Slider> pMaxElevSlider = std::make_shared<Slider>(glm::vec2{850, 150}, 250);
	pMaxElevSlider->setLabel("Max Elevation");
	pMaxElevSlider->setRange(10, 100, 0.5f);
	pMaxElevSlider->setDisplayStyle(5, 1, 0);
	pMaxElevSlider->onValueChanged.add(std::bind(&SessionSetupHostMenu::onMaxElevationChanged, this, pMaxElevLabel, std::placeholders::_1));
	pMaxElevSlider->setValue(pData_->maxElevation);
	addElement(pMaxElevSlider);

	pTerrainPicture_ = std::make_shared<Picture>(glm::vec2{25, 100}, glm::vec2{650, 500});
	pTerrainPicture_->onStartDrag.forward(onTerrainStartDrag);
	pTerrainPicture_->onEndDrag.forward(onTerrainEndDrag);
	pTerrainPicture_->onDrag.forward(onTerrainDrag);
	pTerrainPicture_->onScroll.forward(onTerrainZoom);
	addElement(pTerrainPicture_);
}

void SessionSetupHostMenu::setRTTexture(int texId) {
	pTerrainPicture_->setPictureTexture(texId, false);
}

glm::vec2 SessionSetupHostMenu::terrainPictureSize() const {
	return pTerrainPicture_->getSize();
}

void SessionSetupHostMenu::onMinElevationChanged(std::shared_ptr<Label> label, float value) {
	pData_->minElevation = value;
	std::stringstream ss;
	ss << std::fixed << std::setprecision(1) << value;
	label->setText(ss.str());

	onParametersChanged.trigger();
}

void SessionSetupHostMenu::onMaxElevationChanged(std::shared_ptr<Label> label, float value) {
	pData_->maxElevation = value;
	std::stringstream ss;
	ss << std::fixed << std::setprecision(1) << value;
	label->setText(ss.str());

	onParametersChanged.trigger();
}
