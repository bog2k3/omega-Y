#include "ISODL_Object.h"
#include "strbld.h"

#include <boglfw/utils/assert.h>

#include <algorithm>

// constructs a descriptor for a simple value type property
SODL_Property_Descriptor::SODL_Property_Descriptor(SODL_Value::Type valueType, void* valuePtr)
	: isObject(false), type(valueType), valueOrCallbackPtr(valuePtr) {
}

// ctor for an enum value
SODL_Property_Descriptor::SODL_Property_Descriptor(const char* enumType, int32_t &valuePtr)
	: SODL_Property_Descriptor(SODL_Value::Type::Enum, &valuePtr) {
	this->enumName = enumType;
}

// constructs a descriptor for an object type property
SODL_Property_Descriptor::SODL_Property_Descriptor(const char* objectType, std::shared_ptr<ISODL_Object> &objectPtr)
	: isObject(true), objectType(objectType)
	, valueOrCallbackPtr(&objectPtr) {
}

// constructs a descriptor for an object type property that will be set via the provided callback
SODL_Property_Descriptor::SODL_Property_Descriptor(const char* objectType, std::function<bool(std::shared_ptr<ISODL_Object>)> setObjPropCallback)
	: isObject(true), objectType(objectType)
	, valueOrCallbackPtr(nullptr)
	, pCallbackWrapper(new CallbackWrapperModel::Instance<std::shared_ptr<ISODL_Object>>(setObjPropCallback)) {
}

SODL_Property_Descriptor::SODL_Property_Descriptor(const char* enumType, std::function<bool(int32_t)> setValueCallback)
	: SODL_Property_Descriptor(SODL_Value::Type::Enum, (void*)nullptr) {
	this->enumName = enumType;
	pCallbackWrapper.reset(new CallbackWrapperModel::Instance<int32_t>(setValueCallback));
}

// constructs a descriptor for a callback (std::function<void(argTypes...)>)
SODL_Property_Descriptor::SODL_Property_Descriptor(void* funcPtr, std::vector<SODL_Value::Type> argTypes, bool isEvent)
	: isObject(false), isEvent(isEvent)
	, type(SODL_Value::Type::Callback)
	, callbackArgTypes(argTypes), valueOrCallbackPtr(funcPtr) {
}

void ISODL_Object::defineEnum(const char* enumName, std::vector<std::string> enumLabels) {
	userEnums_[enumName].swap(enumLabels);
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
	return setProperty(desc.name, val);
}

SODL_result ISODL_Object::setProperty(std::string const& propName, SODL_Value const& val) {
	auto it = mapPropertyDesc_.find(propName);
	if (it == mapPropertyDesc_.end())
		return SODL_result::error(strbld() << "Property with name '" << propName << "' not defined in object of type '" << objectType() << "'");
	auto &desc = it->second;
	bool success = true;
	switch (desc.type)
	{
	case SODL_Value::Type::Coordinate: {
		FlexCoord coordVal {val.numberVal, val.isPercentCoord ? FlexCoord::PERCENT : FlexCoord::PIXELS};
		if (desc.valueOrCallbackPtr)
			*static_cast<FlexCoord*>(desc.valueOrCallbackPtr) = coordVal;
		else {
			assertDbg(desc.pCallbackWrapper != nullptr);
			success = desc.pCallbackWrapper->applyCallback(&coordVal);
		}
	} break;
	case SODL_Value::Type::Enum: {
		int32_t enumVal;
		auto res = resolveEnumValue(desc.enumName, val.enumVal, enumVal);
		if (!res)
			return res;
		if (desc.valueOrCallbackPtr)
			*static_cast<int32_t*>(desc.valueOrCallbackPtr) = enumVal;
		else {
			assertDbg(desc.pCallbackWrapper != nullptr);
			success = desc.pCallbackWrapper->applyCallback(&enumVal);
		}
	} break;
	case SODL_Value::Type::Number:
		if (desc.valueOrCallbackPtr)
			*static_cast<float*>(desc.valueOrCallbackPtr) = val.numberVal;
		else {
			assertDbg(desc.pCallbackWrapper != nullptr);
			success = desc.pCallbackWrapper->applyCallback(&val.numberVal);
		}
		break;
	case SODL_Value::Type::String:
		if (desc.valueOrCallbackPtr)
			*static_cast<std::string*>(desc.valueOrCallbackPtr) = val.stringVal;
		else {
			assertDbg(desc.pCallbackWrapper != nullptr);
			success = desc.pCallbackWrapper->applyCallback(&val.stringVal);
		}
		break;
	case SODL_Value::Type::Bool:
		if (desc.valueOrCallbackPtr)
			*static_cast<bool*>(desc.valueOrCallbackPtr) = val.boolVal;
		else {
			assertDbg(desc.pCallbackWrapper != nullptr);
			success = desc.pCallbackWrapper->applyCallback(&val.boolVal);
		}
		break;
	case SODL_Value::Type::Callback:
		success = false;
	default:
		return SODL_result::error(strbld() << "Unhandled value type : " << (int)desc.type);
	}
	if (!success)
		return SODL_result::error(strbld() << "Failed to set property '" << desc.name << "' via callback for object type '" << objectType() << "'");
	else
		return SODL_result::OK();
}

SODL_result ISODL_Object::setProperty(std::string const& propName, std::shared_ptr<ISODL_Object> objPtr) {
	auto it = mapPropertyDesc_.find(propName);
	if (it == mapPropertyDesc_.end())
		return SODL_result::error(strbld() << "Property with name '" << propName << "' not defined in object of type '" << objectType() << "'");
	auto &desc = it->second;
	assertDbg(desc.isObject);
	assertDbg(objPtr != nullptr);
	if (desc.valueOrCallbackPtr != nullptr)
		*static_cast<std::shared_ptr<ISODL_Object>*>(desc.valueOrCallbackPtr) = objPtr;
	else {
		assertDbg(desc.pCallbackWrapper != nullptr);
		if (!desc.pCallbackWrapper->applyCallback(&objPtr))
			return SODL_result::error(strbld() << "Failed to set property '" << propName << "' via callback for object type '" << objectType() << "'");
	}
	return SODL_result::OK();
}

SODL_result ISODL_Object::describePrimaryProperty(unsigned index, SODL_Property_Descriptor &out_desc) {
	if (index >= primaryPropertyDesc_.size())
		return SODL_result::error(strbld() << "Invalid primary property index: " << index << " for object type '" << objectType() << "'");
	out_desc = primaryPropertyDesc_[index];
	return SODL_result::OK();
}

SODL_result ISODL_Object::describeProperty(std::string const& propName, SODL_Property_Descriptor &out_desc) {
	auto it = mapPropertyDesc_.find(propName);
	if (it == mapPropertyDesc_.end())
		return SODL_result::error(strbld() << "Unknown property '" << propName << "' for object type '" << objectType() << "'");
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
		return SODL_result::error(strbld() << "Enum value '" << identName << "' doesnt exist in enum '" << enumName << "' within object type '" << objectType() << "'");
}

void ISODL_Object::defineSupportedChildTypes(std::vector<std::string> childTypes) {
	childTypes_.swap(childTypes);
}

bool ISODL_Object::supportsChildType(std::string const& objType) {
	return std::find(childTypes_.begin(), childTypes_.end(), objType) != childTypes_.end();
}
