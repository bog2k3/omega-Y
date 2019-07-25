#pragma once

#include "ISODL_Object.h"

// helper classes ------------------------------------------------------------

class SODL_Property_Descriptor::CallbackWrapperModel {
public:
	virtual ~CallbackWrapperModel() = default;

	virtual bool applyCallback(const void* pVal) = 0;

	template<class valueType>
	class Instance;
};

template<class valueType>
class SODL_Property_Descriptor::CallbackWrapperModel::Instance : public SODL_Property_Descriptor::CallbackWrapperModel {
public:
	Instance(std::function<bool(valueType)> callback)
		: callback_(callback) {
	}
	~Instance() = default;

	bool applyCallback(const void* pVal) override;

private:
	std::function<bool(valueType)> callback_;
};

template<>
inline bool SODL_Property_Descriptor::CallbackWrapperModel::Instance<float>::applyCallback(const void* pVal) {
	return callback_(*static_cast<const float*>(pVal));
}

template<>
inline bool SODL_Property_Descriptor::CallbackWrapperModel::Instance<FlexCoord>::applyCallback(const void* pVal) {
	return callback_(*static_cast<const FlexCoord*>(pVal));
}

template<>
inline bool SODL_Property_Descriptor::CallbackWrapperModel::Instance<int32_t>::applyCallback(const void* pVal) {
	return callback_(*static_cast<const int32_t*>(pVal));
}

template<>
inline bool SODL_Property_Descriptor::CallbackWrapperModel::Instance<std::string>::applyCallback(const void* pVal) {
	return callback_(*static_cast<const std::string*>(pVal));
}

template<>
inline bool SODL_Property_Descriptor::CallbackWrapperModel::Instance<std::shared_ptr<ISODL_Object>>::applyCallback(const void* pVal) {
	return callback_(*static_cast<const std::shared_ptr<ISODL_Object>*>(pVal));
}

// direct value ref ctors  ------------------------------------------------------------

template<>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(float &numberValuePtr, ...)
	: SODL_Property_Descriptor(SODL_Value::Type::Number, &numberValuePtr) {}

template<>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(FlexibleCoordinate &coordValuePtr, ...)
	: SODL_Property_Descriptor(SODL_Value::Type::Coordinate, &coordValuePtr) {}

template<>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(std::string &stringValuePtr, ...)
	: SODL_Property_Descriptor(SODL_Value::Type::String, &stringValuePtr) {}

template<>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(bool &boolValuePtr, ...)
	: SODL_Property_Descriptor(SODL_Value::Type::Bool, &boolValuePtr) {}

// value callbacks ctors ------------------------------------------------------------

template<>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(std::function<bool(float)> setValueCallback)
	: SODL_Property_Descriptor(SODL_Value::Type::Number, (void*)nullptr) {
	pCallbackWrapper.reset(new CallbackWrapperModel::Instance<float>(setValueCallback));
}

template<>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(std::function<bool(FlexCoord)> setValueCallback)
	: SODL_Property_Descriptor(SODL_Value::Type::Coordinate, (void*)nullptr) {
	pCallbackWrapper.reset(new CallbackWrapperModel::Instance<FlexCoord>(setValueCallback));
}

template<>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(std::function<bool(std::string)> setValueCallback)
	: SODL_Property_Descriptor(SODL_Value::Type::String, (void*)nullptr) {
	pCallbackWrapper.reset(new CallbackWrapperModel::Instance<std::string>(setValueCallback));
}

template<>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(std::function<bool(bool)> setValueCallback)
	: SODL_Property_Descriptor(SODL_Value::Type::Bool, (void*)nullptr) {
	pCallbackWrapper.reset(new CallbackWrapperModel::Instance<bool>(setValueCallback));
}

// callback and event ctors ------------------------------------------------------------

template<class FuncType>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(std::function<FuncType> &funcPtr, std::vector<SODL_Value::Type> argTypes)
	: SODL_Property_Descriptor((void*)&funcPtr, argTypes, false) {}

template<class HandlerFuncType>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(Event<HandlerFuncType> &event, std::vector<SODL_Value::Type> argTypes)
	: SODL_Property_Descriptor((void*)&event, argTypes, true) {}

template<class C, class D>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(C &c, ...) {
	static_assert(std::is_convertible<C, void>(), "Property type not allowed");
}
