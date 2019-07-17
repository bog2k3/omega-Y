#include "ISODL_Object.h"
#include "strbld.h"

#include <boglfw/utils/assert.h>

// constructs a descriptor for a simple value type property
SODL_Property_Descriptor::SODL_Property_Descriptor(SODL_Value::Type valueType, void* valuePtr)
	: isObject(false), type(valueType), valueOrCallbackPtr(valuePtr) {
}

SODL_Property_Descriptor::SODL_Property_Descriptor(const char* enumType, int32_t &valuePtr)
	: SODL_Property_Descriptor(SODL_Value::Type::Enum, &valuePtr) {
	this->enumName = enumType;
}

// constructs a descriptor for an object type property
SODL_Property_Descriptor::SODL_Property_Descriptor(const char* objectType, std::shared_ptr<ISODL_Object> *objectPtr)
	: isObject(true), valueOrCallbackPtr(objectPtr) {
	objectTypes.push_back({objectType, objectType});
}

// constructs a descriptor for an object type property that can accept one of multiple object types
SODL_Property_Descriptor::SODL_Property_Descriptor(std::vector<std::pair<std::string, std::string>> objectTypes, std::shared_ptr<ISODL_Object> *objectPtr)
	: isObject(true), objectTypes(objectTypes), valueOrCallbackPtr(objectPtr) {
}

// constructs a descriptor for a callback (std::function<void(argTypes...)>)
SODL_Property_Descriptor::SODL_Property_Descriptor(void* funcPtr, std::vector<SODL_Value::Type> argTypes, bool isEvent)
	: isObject(false), isEvent(isEvent)
	, type(SODL_Value::Type::Callback)
	, callbackArgTypes(argTypes), valueOrCallbackPtr(funcPtr) {
}

void ISODL_Object::defineEnum(const char* enumName, std::vector<std::string> enumLabels) {
	userEnums_[enumName] = enumLabels;
}

void ISODL_Object::definePrimaryProperty(const char* name, SODL_Property_Descriptor descriptor) {
	descriptor.name = name;
	primaryPropertyDesc_.push_back(descriptor);
	mapPropertyDesc_[name] = descriptor;
}

// defines a "secondary" (optional) property; these must be defined as name: value within the element's block
void ISODL_Object::defineSecondaryProperty(const char* name, SODL_Property_Descriptor desc) {
	desc.name = name;
	mapPropertyDesc_[name] = desc;
}

SODL_result ISODL_Object::setPrimaryProperty(unsigned index, SODL_Value const& val) {
	assertDbg(index < primaryPropertyDesc_.size());
	auto &desc = primaryPropertyDesc_[index];
	assertDbg(!desc.isObject);
	assertDbg(!val.isBinding);
	bool success = true;
	switch (desc.type)
	{
	case SODL_Value::Type::Coordinate: {
		FlexCoord coordVal {val.numberVal, val.isPercentCoord ? FlexCoord::PERCENT : FlexCoord::PIXELS};
		if (desc.valueOrCallbackPtr)
			*static_cast<FlexCoord*>(desc.valueOrCallbackPtr) = coordVal;
		else
			success = setUserPropertyValue(desc.name.c_str(), coordVal);
	} break;
	case SODL_Value::Type::Enum: {
		int32_t enumVal;
		auto res = resolveEnumValue(desc.enumName, val.enumVal, enumVal);
		if (!res)
			return res;
		if (desc.valueOrCallbackPtr)
			*static_cast<int32_t*>(desc.valueOrCallbackPtr) = enumVal;
		else
			success = setUserPropertyValue(desc.name.c_str(), enumVal);
	} break;
	case SODL_Value::Type::Number:
	case SODL_Value::Type::String:
	case SODL_Value::Type::Callback:
		success = false;
	default:
		return SODL_result::error(strbld() << "Unhandled value type : " << (int)desc.type);
	}
	if (!success)
		return SODL_result::error(strbld() << "Failed to set user property '" << desc.name << "' for object type '" << objectType() << "'");
	else
		return SODL_result::OK();
}

SODL_result ISODL_Object::instantiateClass(std::string const& className, std::shared_ptr<ISODL_Object> &out_pInstance) {
	return SODL_result::error("not implemented");
}

SODL_result ISODL_Object::addChildObject(std::shared_ptr<ISODL_Object> pObj) {
	return SODL_result::error("not implemented");
}

SODL_result ISODL_Object::setProperty(std::string const& propName, SODL_Value const& val) {
	return SODL_result::error("not implemented");
}

SODL_result ISODL_Object::setProperty(std::string const& propName, std::shared_ptr<ISODL_Object> objPtr) {
	return SODL_result::error("not implemented");
}

SODL_result ISODL_Object::describePrimaryProperty(unsigned index, SODL_Property_Descriptor &out_desc) {
	if (index >= primaryPropertyDesc_.size())
		return SODL_result::error(strbld() << "Invalid primary property index: " << index);
	out_desc = primaryPropertyDesc_[index];
	return SODL_result::OK();
}

SODL_result ISODL_Object::describeProperty(std::string const& propName, SODL_Property_Descriptor &out_desc) {
	auto it = mapPropertyDesc_.find(propName);
	if (it == mapPropertyDesc_.end())
		return SODL_result::error(strbld() << "Unknown property \"" << propName << "\"");
	out_desc = it->second;
	return SODL_result::OK();
}

SODL_result ISODL_Object::resolveEnumValue(std::string const& enumName, std::string const& identName, int32_t &out_val) {
	auto it = userEnums_.find(enumName);
	if (it == userEnums_.end())
		return SODL_result::error(strbld() << "Enum with name '" << enumName << "' was not defined for object type '" << objectType() << "'");
	out_val = -1;
	for (unsigned i=0; i<it->second.size(); i++) {
		if (it->second[i] == identName) {
			out_val = i;
			break;
		}
	}
	if (out_val >= 0)
		return SODL_result::OK();
	else
		return SODL_result::error(strbld() << "Enum value '" << identName << "' doesnt exist in enum '" << enumName << "' within object type '" << objectType << "'");
}
