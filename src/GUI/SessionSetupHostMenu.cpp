#include "SessionSetupHostMenu.h"

#include "../sodl/SODL_loader.h"
#include "../sodl/GUI_SODL_OBJ_Factory.h"
//#include "../sodl/wrappers/common/Coord2SODLWrapper.h"
//#include "../sodl/wrappers/common/Coord4SODLWrapper.h"
//#include "../sodl/wrappers/GUI/GuiElementSODLWrapper.h"
//#include "../sodl/wrappers/GUI/ButtonSODLWrapper.h"
#include "../sodl/wrappers/GUI/ContainerSODLWrapper.h"
#include "../sodl/wrappers/GUI/LabelSODLWrapper.h"
#include "../sodl/wrappers/GUI/PictureSODLWrapper.h"
#include "../sodl/wrappers/GUI/SliderSODLWrapper.h"
#include "../sodl/wrappers/GUI/TextFieldSODLWrapper.h"
//#include "../sodl/wrappers/GUI/LayoutSODLWrapper.h"

#include <boglfw/GUI/controls/Button.h>
#include <boglfw/GUI/controls/Label.h>
#include <boglfw/GUI/controls/TextField.h>
#include <boglfw/GUI/controls/Picture.h>
#include <boglfw/GUI/controls/Slider.h>
#include <boglfw/GUI/GridLayout.h>
#include <boglfw/utils/rand.h>
#include <boglfw/utils/log.h>

#include <sstream>
#include <iomanip>

struct SessionSetupHostMenu::Controls {
	std::shared_ptr<Picture> pTerrainPicture_;
	std::shared_ptr<TextField> pSeedField_;
	std::shared_ptr<Slider> pMinElevSlider_;
	std::shared_ptr<Slider> pMaxElevSlider_;
	std::shared_ptr<Slider> pRoughnessSlider_;
	std::shared_ptr<Label> pMinElevDisplay_;
	std::shared_ptr<Label> pMaxElevDisplay_;
	std::shared_ptr<Label> pRoughnessDisplay_;
};

SessionSetupHostMenu::SessionSetupHostMenu(TerrainConfig* pData)
	: pControls_(new Controls())
	, pData_(pData)
{
	GUI_SODL_OBJ_Factory objFactory;
	SODL_Loader loader(objFactory);

	loader.addDataBinding("terrainSeed", pData->seed);
	loader.addDataBinding("minElevation", pData->minElevation);
	loader.addDataBinding("maxElevation", pData->maxElevation);
	loader.addDataBinding("roughness", pData->roughness);

	loader.addActionBinding<void()>("seedChanged", {}, std::bind(std::bind(&SessionSetupHostMenu::onSeedChanged, this)));
	loader.addActionBinding<void(float)>("minElevationChanged", {SODL_Value::Type::Number},
		std::bind(&SessionSetupHostMenu::onTerrainParameterChanged, this,
		&pData_->minElevation, std::placeholders::_1, &pControls_->pMinElevDisplay_));
	loader.addActionBinding<void(float)>("maxElevationChanged", {SODL_Value::Type::Number},
		std::bind(&SessionSetupHostMenu::onTerrainParameterChanged, this,
		&pData_->maxElevation, std::placeholders::_1, &pControls_->pMaxElevDisplay_));
	loader.addActionBinding<void(float)>("roughnessChanged", {SODL_Value::Type::Number},
		std::bind(&SessionSetupHostMenu::onTerrainParameterChanged, this,
		&pData_->roughness, std::placeholders::_1, &pControls_->pRoughnessDisplay_));
	loader.addActionBinding<void()>("randSeed", {}, std::bind(&SessionSetupHostMenu::onRandSeed, this));
	loader.addActionBinding<void()>("randomizeAll", {}, std::bind(&SessionSetupHostMenu::onRandomizeAll, this));
	loader.addActionBinding<void()>("goBack", {}, onBack);
	loader.addActionBinding<void()>("startGame", {}, onStart);

	std::shared_ptr<ContainerSODLWrapper> pRoot(this);
	auto res = loader.mergeObject(*pRoot, "data/ui/sessionSetupHostMenu.sodl");
	if (!res) {
		ERROR("Failed to load SessionSetupHostMenu SODL: " << res.errorMessage);
		return;
	}

	/* setSize({100, 100, FlexCoord::PERCENT});
	setClientArea({50, 50}, {50, 50});
	auto layout = std::make_shared<GridLayout>();
	useLayout(layout);

	//glm::vec2{mySize.x / 2 - 125, 85},
	std::shared_ptr<Label> pTitle = std::make_shared<Label>(50, "Host Game");
	addElement(pTitle);

	//glm::vec2{10, mySize.y - 60}, glm::vec2{200, 50},
	std::shared_ptr<Button> pBack = std::make_shared<Button>("Back");
	pBack->onClick.forward(onBack);
	addElement(pBack);

	//glm::vec2{mySize.x - 210, mySize.y - 60}, glm::vec2{200, 50},
	std::shared_ptr<Button> pStart = std::make_shared<Button>("Start");
	pStart->onClick.forward(onStart);
	addElement(pStart);

	//glm::vec2{700, 140},
	std::shared_ptr<Label> pSeedLabel = std::make_shared<Label>(18, "Seed");
	addElement(pSeedLabel);

	//glm::vec2{780, 110}, glm::vec2{150, 30},
	std::shared_ptr<TextField> pSeedField = std::make_shared<TextField>(TextField::NUMBER, std::to_string(pData->seed));
	pSeedField->onChanged.add(std::bind(&SessionSetupHostMenu::onSeedChanged, this, pSeedField));
	addElement(pSeedField);

	//glm::vec2{950, 110}, glm::vec2{100, 30},
	std::shared_ptr<Button> pRandSeed = std::make_shared<Button>("Random");
	pRandSeed->onClick.add(std::bind(&SessionSetupHostMenu::onRandSeed, this, pSeedField));
	addElement(pRandSeed);

	constexpr float slidersStartY = 200;
	constexpr float slidersSpacing = 60;

	//glm::vec2{1120, slidersStartY + 30},
	std::shared_ptr<Label> pMinElevDisplay = std::make_shared<Label>(18, "");
	addElement(pMinElevDisplay);

	//glm::vec2{850, slidersStartY}, 250
	std::shared_ptr<Slider> pMinElevSlider = std::make_shared<Slider>();
	pMinElevSlider->setLabel("Min Elevation");
	pMinElevSlider->setRange(-20, -1, 0.5f);
	pMinElevSlider->setDisplayStyle(5, 1, 0);
	pMinElevSlider->onValueChanged.add(std::bind(&SessionSetupHostMenu::onTerrainParameterChanged, this,
		&pData_->minElevation, std::placeholders::_1, pMinElevDisplay));
	pMinElevSlider->setValue(pData_->minElevation);
	addElement(pMinElevSlider);

	//glm::vec2{1120, slidersStartY + slidersSpacing * 1 + 30},
	std::shared_ptr<Label> pMaxElevDisplay = std::make_shared<Label>(18, "");
	addElement(pMaxElevDisplay);

	//glm::vec2{850, slidersStartY + slidersSpacing * 1}, 250
	std::shared_ptr<Slider> pMaxElevSlider = std::make_shared<Slider>();
	pMaxElevSlider->setLabel("Max Elevation");
	pMaxElevSlider->setRange(10, 50, 0.5f);
	pMaxElevSlider->setDisplayStyle(5, 2, 0);
	pMaxElevSlider->onValueChanged.add(std::bind(&SessionSetupHostMenu::onTerrainParameterChanged, this,
		&pData_->maxElevation, std::placeholders::_1, pMaxElevDisplay));
	pMaxElevSlider->setValue(pData_->maxElevation);
	addElement(pMaxElevSlider);

	//glm::vec2{1120, slidersStartY + slidersSpacing * 2 + 30},
	std::shared_ptr<Label> pRoughnessDisplay = std::make_shared<Label>(18, "");
	addElement(pRoughnessDisplay);

	//glm::vec2{850, slidersStartY + slidersSpacing * 2}, 250
	std::shared_ptr<Slider> pRoughnessSlider = std::make_shared<Slider>();
	pRoughnessSlider->setLabel("Roughness");
	pRoughnessSlider->setRange(0.f, 1.f, 0.1f);
	pRoughnessSlider->setDisplayStyle(0.1, 2, 1);
	pRoughnessSlider->onValueChanged.add(std::bind(&SessionSetupHostMenu::onTerrainParameterChanged, this,
		&pData_->roughness, std::placeholders::_1, pRoughnessDisplay));
	pRoughnessSlider->setValue(pData_->roughness);
	addElement(pRoughnessSlider);

	//glm::vec2{700, slidersStartY + slidersSpacing * 3.5f}, glm::vec2{450, 30},
	std::shared_ptr<Button> pRandomize = std::make_shared<Button>("Randomize all");
	pRandomize->onClick.add(std::bind(&SessionSetupHostMenu::onRandomizeAll, this,
		pSeedField, pMinElevSlider, pMaxElevSlider, pRoughnessSlider));
	addElement(pRandomize);

#ifdef DEBUG
	//glm::vec2{1120, slidersStartY + slidersSpacing * 5 + 30},
	std::shared_ptr<Label> pDensityDisplay = std::make_shared<Label>(18, "");
	addElement(pDensityDisplay);

	//glm::vec2{850, slidersStartY + slidersSpacing * 5}, 250
	std::shared_ptr<Slider> pVertexDensitySlider = std::make_shared<Slider>();
	pVertexDensitySlider->setLabel("Vertex density");
	pVertexDensitySlider->setRange(0.5, 2.0, 0.1f);
	pVertexDensitySlider->setDisplayStyle(0.1, 4, 1);
	pVertexDensitySlider->onValueChanged.add(std::bind(&SessionSetupHostMenu::onTerrainParameterChanged, this,
		&pData_->vertexDensity, std::placeholders::_1, pDensityDisplay));
	pVertexDensitySlider->setValue(pData_->vertexDensity);
	addElement(pVertexDensitySlider);
#endif

	//glm::vec2{25, 100}, glm::vec2{650, 500}
	pTerrainPicture_ = std::make_shared<Picture>();
	pTerrainPicture_->onStartDrag.forward(onTerrainStartDrag);
	pTerrainPicture_->onEndDrag.forward(onTerrainEndDrag);
	pTerrainPicture_->onDrag.forward(onTerrainDrag);
	pTerrainPicture_->onScroll.forward(onTerrainZoom);
	addElement(pTerrainPicture_);*/
}

SessionSetupHostMenu::~SessionSetupHostMenu() {
	delete pControls_;
}

void SessionSetupHostMenu::setRTTexture(int texId) {
	pTerrainPicture_->setPictureTexture(texId, false);
}

glm::vec2 SessionSetupHostMenu::terrainPictureSize() const {
	return pTerrainPicture_->computedSize();
}

void SessionSetupHostMenu::onTerrainParameterChanged(float* pParam, float value, std::shared_ptr<Label> *label) {
	assertDbg(pParam);
	*pParam = value;
	std::stringstream ss;
	ss << std::fixed << std::setprecision(1) << value;
	label->->setText(ss.str());

	onParametersChanged.trigger();
}

void SessionSetupHostMenu::onSeedChanged() {

}

void SessionSetupHostMenu::onRandSeed() {
	pData_->seed = new_RID();
	pSeedField_->setText(std::to_string(pData_->seed));
	onParametersChanged.trigger();
}

void SessionSetupHostMenu::onRandomizeAll()
{
	onRandSeed();

	auto minElRange = pMinElevSlider_->getRange();
	pMinElevSlider_->setValue(minElRange.first + (minElRange.second - minElRange.first) * randf());

	auto maxElRange = pMaxElevSlider_->getRange();
	pMaxElevSlider_->setValue(maxElRange.first + (maxElRange.second - maxElRange.first) * randf());

	auto roughRange = pRoughnessSlider_->getRange();
	pRoughnessSlider_->setValue(roughRange.first + (roughRange.second - roughRange.first) * randf());
}
