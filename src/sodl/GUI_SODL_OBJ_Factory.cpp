#include "GUI_SODL_OBJ_Factory.h"
#include "wrappers/common/Coord2SODLWrapper.h"
#include "wrappers/common/Coord4SODLWrapper.h"
#include "wrappers/GUI/GuiElementSODLWrapper.h"
#include "wrappers/GUI/ButtonSODLWrapper.h"
#include "wrappers/GUI/ContainerSODLWrapper.h"
#include "wrappers/GUI/LabelSODLWrapper.h"
#include "wrappers/GUI/PictureSODLWrapper.h"
#include "wrappers/GUI/SliderSODLWrapper.h"
#include "wrappers/GUI/TextFieldSODLWrapper.h"
#include "wrappers/GUI/LayoutSODLWrapper.h"
#include <boglfw/utils/strbld.h>

GUI_SODL_OBJ_Factory::~GUI_SODL_OBJ_Factory() {
	for (auto &p : mapClassDef_)
		delete p.second.creator_;
	mapClassDef_.clear();
}

SODL_result GUI_SODL_OBJ_Factory::constructObject(std::string const& objType, std::shared_ptr<ISODL_Object> &outObj) {
	auto it = mapClassDef_.find(objType);
	if (it == mapClassDef_.end())
		return SODL_result::error(strbld() << "Unknown object type: '" << objType << "'");
	if (it->second.metadata_.isAbstract)
		return SODL_result::error(strbld() << "Cannot instantiate abstract type: '" << objType << "'");
	outObj = it->second.creator_->create();
	return SODL_result::OK();
}

SODL_result GUI_SODL_OBJ_Factory::getTypeInfo(std::string const& typeName, SODL_ObjectTypeDescriptor &outInfo) {
	auto it = mapClassDef_.find(typeName);
	if (it == mapClassDef_.end())
		return SODL_result::error("Unknown object type: " + typeName);
	outInfo = it->second.metadata_;
	return SODL_result::OK();
}

GUI_SODL_OBJ_Factory::GUI_SODL_OBJ_Factory() {
	addClassDefinition<Coord2SODLWrapper>();
	addClassDefinition<Coord4SODLWrapper>();
	addAbstractClassDefinition<GuiElementSODLWrapper>();
	addClassDefinition<LabelSODLWrapper>();
	addClassDefinition<PictureSODLWrapper>();
	addClassDefinition<ContainerSODLWrapper>();
	addClassDefinition<TextFieldSODLWrapper>();
	addClassDefinition<SliderSODLWrapper>();
	addClassDefinition<ButtonSODLWrapper>();
	addClassDefinition<LayoutSODLWrapper>();
}
