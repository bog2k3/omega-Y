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
}

SessionSetupHostMenu::~SessionSetupHostMenu() {
	delete pControls_;
}

void SessionSetupHostMenu::load() {
	GUI_SODL_OBJ_Factory objFactory;
	SODL_Loader loader(objFactory);

	loader.addDataBinding("terrainSeed", pData_->seed);
	loader.addDataBinding("minElevation", pData_->minElevation);
	loader.addDataBinding("maxElevation", pData_->maxElevation);
	loader.addDataBinding("roughness", pData_->roughness);

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
	loader.addActionBinding<>("goBack", {}, onBack);
	loader.addActionBinding<>("startGame", {}, onStart);

	ContainerSODLWrapper root(*this);
	auto res = loader.mergeObject(root, "data/ui/sessionSetupHostMenu.sodl");
	if (!res) {
		ERROR("Failed to load SessionSetupHostMenu SODL: " << res.errorMessage);
		return;
	}
	pControls_->pMaxElevDisplay_ = root.getElement<Label>("maxElevDisp");
	pControls_->pMinElevDisplay_ = root.getElement<Label>("minElevDisp");
	pControls_->pRoughnessDisplay_ = root.getElement<Label>("roughDisp");
	pControls_->pMaxElevSlider_ = root.getElement<Slider>("maxElev");
	pControls_->pMinElevSlider_ = root.getElement<Slider>("minElev");
	pControls_->pRoughnessSlider_ = root.getElement<Slider>("roughness");
	pControls_->pSeedField_ = root.getElement<TextField>("seed");
	pControls_->pTerrainPicture_ = root.getElement<Picture>("terrainPicture");
	pControls_->pTerrainPicture_->onStartDrag.forward(onTerrainStartDrag);
	pControls_->pTerrainPicture_->onEndDrag.forward(onTerrainEndDrag);
	pControls_->pTerrainPicture_->onDrag.forward(onTerrainDrag);
	pControls_->pTerrainPicture_->onScroll.forward(onTerrainZoom);
}

void SessionSetupHostMenu::setRTTexture(int texId) {
	pControls_->pTerrainPicture_->setPictureTexture(texId, false);
}

glm::vec2 SessionSetupHostMenu::terrainPictureSize() const {
	return pControls_->pTerrainPicture_->computedSize();
}

void SessionSetupHostMenu::onTerrainParameterChanged(float* pParam, float value, std::shared_ptr<Label> *label) {
	assertDbg(pParam);
	*pParam = value;
	std::stringstream ss;
	ss << std::fixed << std::setprecision(1) << value;
	(*label)->setText(ss.str());

	onParametersChanged.trigger();
}

void SessionSetupHostMenu::onSeedChanged() {

}

void SessionSetupHostMenu::onRandSeed() {
	pData_->seed = new_RID();
	pControls_->pSeedField_->seValue(pData_->seed);
	onParametersChanged.trigger();
}

void SessionSetupHostMenu::onRandomizeAll()
{
	onRandSeed();

	auto minElRange = pControls_->pMinElevSlider_->getRange();
	pControls_->pMinElevSlider_->setValue(minElRange.first + (minElRange.second - minElRange.first) * randf());

	auto maxElRange = pControls_->pMaxElevSlider_->getRange();
	pControls_->pMaxElevSlider_->setValue(maxElRange.first + (maxElRange.second - maxElRange.first) * randf());

	auto roughRange = pControls_->pRoughnessSlider_->getRange();
	pControls_->pRoughnessSlider_->setValue(roughRange.first + (roughRange.second - roughRange.first) * randf());
}
