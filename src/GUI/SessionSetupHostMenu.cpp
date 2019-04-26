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
	pMaxElevSlider->setRange(10, 100, 0.5f);
	pMaxElevSlider->setDisplayStyle(5, 1, 0);
	pMaxElevSlider->onValueChanged.add(std::bind(&SessionSetupHostMenu::onTerrainParameterChanged, this,
		&pData_->maxElevation, std::placeholders::_1, pMaxElevDisplay));
	pMaxElevSlider->setValue(pData_->maxElevation);
	addElement(pMaxElevSlider);

	std::shared_ptr<Label> pBigRoughnessDisplay = std::make_shared<Label>(
		glm::vec2{1120, slidersStartY + slidersSpacing * 2 + 30}, 18, "");
	addElement(pBigRoughnessDisplay);

	std::shared_ptr<Slider> pBigRoughnessSlider = std::make_shared<Slider>(
		glm::vec2{850, slidersStartY + slidersSpacing * 2}, 250);
	pBigRoughnessSlider->setLabel("Big roughness");
	pBigRoughnessSlider->setRange(0.5, 1.5, 0.1f);
	pBigRoughnessSlider->setDisplayStyle(11, 2, 1);
	pBigRoughnessSlider->onValueChanged.add(std::bind(&SessionSetupHostMenu::onTerrainParameterChanged, this,
		&pData_->bigRoughness, std::placeholders::_1, pBigRoughnessDisplay));
	pBigRoughnessSlider->setValue(pData_->bigRoughness);
	addElement(pBigRoughnessSlider);

	std::shared_ptr<Label> pSmallRoughnessDisplay = std::make_shared<Label>(
		glm::vec2{1120, slidersStartY + slidersSpacing * 3 + 30}, 18, "");
	addElement(pSmallRoughnessDisplay);

	std::shared_ptr<Slider> pSmallRoughnessSlider = std::make_shared<Slider>(
		glm::vec2{850, slidersStartY + slidersSpacing * 3}, 250);
	pSmallRoughnessSlider->setLabel("Small roughness");
	pSmallRoughnessSlider->setRange(0, 1.0, 0.1f);
	pSmallRoughnessSlider->setDisplayStyle(11, 2, 1);
	pSmallRoughnessSlider->onValueChanged.add(std::bind(&SessionSetupHostMenu::onTerrainParameterChanged, this,
		&pData_->smallRoughness, std::placeholders::_1, pSmallRoughnessDisplay));
	pSmallRoughnessSlider->setValue(pData_->smallRoughness);
	addElement(pSmallRoughnessSlider);

	std::shared_ptr<Button> pRandomize = std::make_shared<Button>(
		glm::vec2{700, slidersStartY + slidersSpacing * 4.5f}, glm::vec2{450, 30}, "Randomize all");
	pRandomize->onClick.add(std::bind(&SessionSetupHostMenu::onRandomizeAll, this,
		pSeedField, pMinElevSlider, pMaxElevSlider, pBigRoughnessSlider, pSmallRoughnessSlider));
	addElement(pRandomize);

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
	std::shared_ptr<Slider> pBigRoughnessSlider, std::shared_ptr<Slider> pSmallRoughnessSlider)
{
	onRandSeed(pSeedField);

	auto minElRange = pMinElevSlider->getRange();
	pMinElevSlider->setValue(minElRange.first + (minElRange.second - minElRange.first) * randf());

	auto maxElRange = pMaxElevSlider->getRange();
	pMaxElevSlider->setValue(maxElRange.first + (maxElRange.second - maxElRange.first) * randf());

	auto bigRoughRange = pBigRoughnessSlider->getRange();
	pBigRoughnessSlider->setValue(bigRoughRange.first + (bigRoughRange.second - bigRoughRange.first) * randf());

	auto smallRoughRange = pSmallRoughnessSlider->getRange();
	pSmallRoughnessSlider->setValue(smallRoughRange.first + (smallRoughRange.second - smallRoughRange.first) * randf());
}
