#pragma once

#include "ISODL_Object.h"

template<>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(float &numberValuePtr)
	: SODL_Property_Descriptor(SODL_Value::Type::Number, &numberValuePtr) {}

template<>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(FlexibleCoordinate &coordValuePtr)
	: SODL_Property_Descriptor(SODL_Value::Type::Coordinate, &coordValuePtr) {}

template<>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(std::string &stringValuePtr)
	: SODL_Property_Descriptor(SODL_Value::Type::String, &stringValuePtr) {}

template<>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(int32_t &enumValuePtr)
	: SODL_Property_Descriptor(SODL_Value::Type::Enum, &enumValuePtr) {}

inline SODL_Property_Descriptor::SODL_Property_Descriptor(SODL_Value::Type propType)
	: SODL_Property_Descriptor(propType, nullptr) {}

template<class FuncType>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(std::function<FuncType> &funcPtr, std::vector<SODL_Value::Type> argTypes)
	: SODL_Property_Descriptor((void*)&funcPtr, argTypes, false) {}

template<class HandlerFuncType>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(Event<HandlerFuncType> &event, std::vector<SODL_Value::Type> argTypes)
	: SODL_Property_Descriptor((void*)&event, argTypes, true) {}

template<class C>
inline SODL_Property_Descriptor::SODL_Property_Descriptor(C& c) {
	static_assert(std::is_convertible<C, void>(), "Property type not allowed");
}
