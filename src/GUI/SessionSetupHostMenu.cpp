#include "SessionSetupHostMenu.h"

#include <boglfw/GUI/controls/Button.h>
#include <boglfw/GUI/controls/Label.h>
#include <boglfw/GUI/controls/TextField.h>
#include <boglfw/GUI/controls/Picture.h>
#include <boglfw/GUI/controls/Slider.h>
#include <boglfw/utils/rand.h>

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

	std::shared_ptr<Button> pStart = std::make_shared<Button>(glm::vec2{mySize.x - 210, mySize.y - 60}, glm::vec2{200, 50}, "Start");
	pStart->onClick.forward(onStart);
	addElement(pStart);

	std::shared_ptr<Label> pSeedLabel = std::make_shared<Label>(glm::vec2{700, 140}, 18, "Seed");
	addElement(pSeedLabel);

	std::shared_ptr<TextField> pSeedField = std::make_shared<TextField>(glm::vec2{780, 110}, glm::vec2{150, 30},
		std::to_string(pData->seed));
	pSeedField->onTextChanged.add(std::bind(&SessionSetupHostMenu::onSeedChanged, this, pSeedField));
	addElement(pSeedField);

	std::shared_ptr<Button> pRandSeed = std::make_shared<Button>(glm::vec2{950, 110}, glm::vec2{100, 30}, "Random");
	pRandSeed->onClick.add(std::bind(&SessionSetupHostMenu::onRandSeed, this, pSeedField));
	addElement(pRandSeed);

	constexpr float slidersStartY = 200;
	constexpr float slidersSpacing = 60;

	std::shared_ptr<Label> pMinElevDisplay = std::make_shared<Label>(
		glm::vec2{1120, slidersStartY + 30}, 18, "");
	addElement(pMinElevDisplay);

	std::shared_ptr<Slider> pMinElevSlider = std::make_shared<Slider>(
		glm::vec2{850, slidersStartY}, 250);
	pMinElevSlider->setLabel("Min Elevation");
	pMinElevSlider->setRange(-20, -1, 0.5f);
	pMinElevSlider->setDisplayStyle(5, 1, 0);
	pMinElevSlider->onValueChanged.add(std::bind(&SessionSetupHostMenu::onTerrainParameterChanged, this,
		&pData_->minElevation, std::placeholders::_1, pMinElevDisplay));
	pMinElevSlider->setValue(pData_->minElevation);
	addElement(pMinElevSlider);

	std::shared_ptr<Label> pMaxElevDisplay = std::make_shared<Label>(
		glm::vec2{1120, slidersStartY + slidersSpacing * 1 + 30}, 18, "");
	addElement(pMaxElevDisplay);

	std::shared_ptr<Slider> pMaxElevSlider = std::make_shared<Slider>(
		glm::vec2{850, slidersStartY + slidersSpacing * 1}, 250);
	pMaxElevSlider->setLabel("Max Elevation");
	pMaxElevSlider->setRange(10, 50, 0.5f);
	pMaxElevSlider->setDisplayStyle(5, 2, 0);
	pMaxElevSlider->onValueChanged.add(std::bind(&SessionSetupHostMenu::onTerrainParameterChanged, this,
		&pData_->maxElevation, std::placeholders::_1, pMaxElevDisplay));
	pMaxElevSlider->setValue(pData_->maxElevation);
	addElement(pMaxElevSlider);

	std::shared_ptr<Label> pRoughnessDisplay = std::make_shared<Label>(
		glm::vec2{1120, slidersStartY + slidersSpacing * 2 + 30}, 18, "");
	addElement(pRoughnessDisplay);

	std::shared_ptr<Slider> pRoughnessSlider = std::make_shared<Slider>(
		glm::vec2{850, slidersStartY + slidersSpacing * 2}, 250);
	pRoughnessSlider->setLabel("Roughness");
	pRoughnessSlider->setRange(0.f, 1.f, 0.1f);
	pRoughnessSlider->setDisplayStyle(0.1, 2, 1);
	pRoughnessSlider->onValueChanged.add(std::bind(&SessionSetupHostMenu::onTerrainParameterChanged, this,
		&pData_->roughness, std::placeholders::_1, pRoughnessDisplay));
	pRoughnessSlider->setValue(pData_->roughness);
	addElement(pRoughnessSlider);

	std::shared_ptr<Button> pRandomize = std::make_shared<Button>(
		glm::vec2{700, slidersStartY + slidersSpacing * 3.5f}, glm::vec2{450, 30}, "Randomize all");
	pRandomize->onClick.add(std::bind(&SessionSetupHostMenu::onRandomizeAll, this,
		pSeedField, pMinElevSlider, pMaxElevSlider, pRoughnessSlider));
	addElement(pRandomize);

#ifdef DEBUG
	std::shared_ptr<Label> pDensityDisplay = std::make_shared<Label>(
		glm::vec2{1120, slidersStartY + slidersSpacing * 5 + 30}, 18, "");
	addElement(pDensityDisplay);

	std::shared_ptr<Slider> pVertexDensitySlider = std::make_shared<Slider>(
		glm::vec2{850, slidersStartY + slidersSpacing * 5}, 250);
	pVertexDensitySlider->setLabel("Vertex density");
	pVertexDensitySlider->setRange(0.5, 2.0, 0.1f);
	pVertexDensitySlider->setDisplayStyle(0.1, 4, 1);
	pVertexDensitySlider->onValueChanged.add(std::bind(&SessionSetupHostMenu::onTerrainParameterChanged, this,
		&pData_->vertexDensity, std::placeholders::_1, pDensityDisplay));
	pVertexDensitySlider->setValue(pData_->vertexDensity);
	addElement(pVertexDensitySlider);
#endif

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

void SessionSetupHostMenu::onTerrainParameterChanged(float* pParam, float value, std::shared_ptr<Label> label) {
	assertDbg(pParam);
	*pParam = value;
	std::stringstream ss;
	ss << std::fixed << std::setprecision(1) << value;
	label->setText(ss.str());

	onParametersChanged.trigger();
}

void SessionSetupHostMenu::onSeedChanged(std::shared_ptr<TextField> pSeedField) {

}

void SessionSetupHostMenu::onRandSeed(std::shared_ptr<TextField> pSeedField) {
	pData_->seed = new_RID();
	pSeedField->setText(std::to_string(pData_->seed));
	onParametersChanged.trigger();
}

void SessionSetupHostMenu::onRandomizeAll(std::shared_ptr<TextField> pSeedField,
	std::shared_ptr<Slider> pMinElevSlider, std::shared_ptr<Slider> pMaxElevSlider,
	std::shared_ptr<Slider> pRoughnessSlider)
{
	onRandSeed(pSeedField);

	auto minElRange = pMinElevSlider->getRange();
	pMinElevSlider->setValue(minElRange.first + (minElRange.second - minElRange.first) * randf());

	auto maxElRange = pMaxElevSlider->getRange();
	pMaxElevSlider->setValue(maxElRange.first + (maxElRange.second - maxElRange.first) * randf());

	auto roughRange = pRoughnessSlider->getRange();
	pRoughnessSlider->setValue(roughRange.first + (roughRange.second - roughRange.first) * randf());
}
