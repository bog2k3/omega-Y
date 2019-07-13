#pragma once

#include "SODL_common.h"

#include <unordered_map>

class GUI_SODL_OBJ_Factory : public ISODL_Object_Factory {
public:
	SODL_result constructObject(std::string const& objType, std::shared_ptr<ISODL_Object> &outObj) override;

	GUI_SODL_OBJ_Factory();

private:
	using createFunc = ISODL_Object* (GUI_SODL_OBJ_Factory::*)();

	std::unordered_map<std::string, createFunc> mapFuncs_;

	ISODL_Object* createCoord2();
	ISODL_Object* createCoord4();

	ISODL_Object* createLabel();
	ISODL_Object* createMarker();
	ISODL_Object* createPicture();
	ISODL_Object* createContainer();
	ISODL_Object* createTextField();
	ISODL_Object* createSlider();
	ISODL_Object* createButton();
};
