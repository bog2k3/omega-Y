#include "GUI_SODL_OBJ_Factory.h"
#include "SODL_wrappers/GUI/ButtonSODLWrapper.h"
#include "SODL_wrappers/GUI/ContainerSODLWrapper.h"
#include "SODL_wrappers/GUI/LabelSODLWrapper.h"
#include "SODL_wrappers/GUI/MarkerSODLWrapper.h"
#include "SODL_wrappers/GUI/PictureSODLWrapper.h"
#include "SODL_wrappers/GUI/SliderSODLWrapper.h"
#include "SODL_wrappers/GUI/TextFieldSODLWrapper.h"

SODL_result GUI_SODL_OBJ_Factory::constructObject(std::string const& objType, ISODL_Object* &outObj) {
	auto it = mapFuncs_.find(objType);
	if (it == mapFuncs_.end())
		return SODL_result::error("Unknown object type: " + objType);
	outObj = (this->*it->second)();
	return SODL_result::OK();
}

GUI_SODL_OBJ_Factory::GUI_SODL_OBJ_Factory() {
	mapFuncs_["label"] = &GUI_SODL_OBJ_Factory::createLabel;
	mapFuncs_["marker"] = &GUI_SODL_OBJ_Factory::createMarker;
	mapFuncs_["picture"] = &GUI_SODL_OBJ_Factory::createPicture;
	mapFuncs_["container"] = &GUI_SODL_OBJ_Factory::createContainer;
	mapFuncs_["textField"] = &GUI_SODL_OBJ_Factory::createTextField;
	mapFuncs_["slider"] = &GUI_SODL_OBJ_Factory::createSlider;
	mapFuncs_["button"] = &GUI_SODL_OBJ_Factory::createButton;
}

ISODL_Object* GUI_SODL_OBJ_Factory::createLabel() {
	return new LabelSODLWrapper();
}

ISODL_Object* GUI_SODL_OBJ_Factory::createMarker() {
	return new MarkerSODLWrapper();
}

ISODL_Object* GUI_SODL_OBJ_Factory::createPicture() {
	return new PictureSODLWrapper();
}

ISODL_Object* GUI_SODL_OBJ_Factory::createContainer() {
	return new ContainerSODLWrapper();
}

ISODL_Object* GUI_SODL_OBJ_Factory::createTextField() {
	return new TextFieldSODLWrapper();
}

ISODL_Object* GUI_SODL_OBJ_Factory::createSlider() {
	return new SliderSODLWrapper();
}

ISODL_Object* GUI_SODL_OBJ_Factory::createButton() {
	return new ButtonSODLWrapper();
}
