#include "GUI_SODL_OBJ_Factory.h"
#include "SODL_wrappers/common/Coord2SODLWrapper.h"
#include "SODL_wrappers/common/Coord4SODLWrapper.h"
#include "SODL_wrappers/GUI/ButtonSODLWrapper.h"
#include "SODL_wrappers/GUI/ContainerSODLWrapper.h"
#include "SODL_wrappers/GUI/LabelSODLWrapper.h"
#include "SODL_wrappers/GUI/MarkerSODLWrapper.h"
#include "SODL_wrappers/GUI/PictureSODLWrapper.h"
#include "SODL_wrappers/GUI/SliderSODLWrapper.h"
#include "SODL_wrappers/GUI/TextFieldSODLWrapper.h"
#include "SODL_wrappers/GUI/layouts/SplitLayoutSODLWrapper.h"
#include "SODL_wrappers/GUI/layouts/ListLayoutSODLWrapper.h"
#include "SODL_wrappers/GUI/layouts/FreeLayoutSODLWrapper.h"
#include "SODL_wrappers/GUI/layouts/FillLayoutSODLWrapper.h"
#include "SODL_wrappers/GUI/layouts/GridLayoutSODLWrapper.h"

GUI_SODL_OBJ_Factory::~GUI_SODL_OBJ_Factory() {
	for (auto &p : mapCreators_)
		delete p.second;
	mapCreators_.clear();
}

SODL_result GUI_SODL_OBJ_Factory::constructObject(std::string const& objType, std::shared_ptr<ISODL_Object> &outObj) {
	auto it = mapCreators_.find(objType);
	if (it == mapCreators_.end())
		return SODL_result::error("Unknown object type: " + objType);
	outObj = it->second->create();
	return SODL_result::OK();
}

GUI_SODL_OBJ_Factory::GUI_SODL_OBJ_Factory() {
	addClassCreator<Coord2SODLWrapper>();
	addClassCreator<Coord4SODLWrapper>();
	addClassCreator<LabelSODLWrapper>();
	addClassCreator<MarkerSODLWrapper>();
	addClassCreator<PictureSODLWrapper>();
	addClassCreator<ContainerSODLWrapper>();
	addClassCreator<TextFieldSODLWrapper>();
	addClassCreator<SliderSODLWrapper>();
	addClassCreator<ButtonSODLWrapper>();
	addClassCreator<SplitLayoutSODLWrapper>();
	addClassCreator<FreeLayoutSODLWrapper>();
	addClassCreator<FillLayoutSODLWrapper>();
	addClassCreator<GridLayoutSODLWrapper>();
	addClassCreator<ListLayoutSODLWrapper>();
}
