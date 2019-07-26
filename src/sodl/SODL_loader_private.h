#pragma once

#include "SODL_loader.h"

#include <memory>
#include <cassert>

class _SODL_Loader_ActionBindingModel {
public:
	virtual ~_SODL_Loader_ActionBindingModel() {}
	virtual void setObjectCallbackBinding(void* fnPtr, bool isEvent) = 0;
};

template<class FuncType>
class _SODL_Loader_ActionBindingWrapper : public _SODL_Loader_ActionBindingModel {
public:
	_SODL_Loader_ActionBindingWrapper(std::function<FuncType> func)
		: func_(func) {
	}
	~_SODL_Loader_ActionBindingWrapper() {}
	void setObjectCallbackBinding(void* fnPtr, bool isEvent) override {
		assertDbg(fnPtr != nullptr);
		if (isEvent)
			static_cast<Event<FuncType>*>(fnPtr)->add(func_);
		else
			*static_cast<std::function<FuncType>*>(fnPtr) = func_;
	}
private:
	std::function<FuncType> func_;
};

struct SODL_Loader::_SODL_Loader_ActionBindingDescriptor {
	std::unique_ptr<_SODL_Loader_ActionBindingModel> pBindingWrapper_;
	std::vector<SODL_Value::Type> argTypes_;
};

template <class FuncType>
void SODL_Loader::addActionBinding(const char* name, std::vector<SODL_Value::Type> argumentTypes, std::function<FuncType> func) {
	assert(mapActionBindings_[name] == nullptr && "Action binding with this name already exists");
	mapActionBindings_[name] = new _SODL_Loader_ActionBindingDescriptor {
		std::unique_ptr<_SODL_Loader_ActionBindingModel>(new _SODL_Loader_ActionBindingWrapper<FuncType>(func)),
		argumentTypes
	};
}

template<class... EventArgs>
void SODL_Loader::addActionBinding(const char* name, std::vector<SODL_Value::Type> argTypes, Event<void(EventArgs...)> &event) {
	addActionBinding<void(EventArgs...)>(name, argTypes, [&event](EventArgs... args) {
		event.trigger(args...);
	});
}

template<>
inline void SODL_Loader::addDataBinding<float>(const char* name, float &data) {
	addDataBindingImpl(name, SODL_Value::Type::Number, &data);
}

template<>
inline void SODL_Loader::addDataBinding<FlexCoord>(const char* name, FlexCoord &data) {
	addDataBindingImpl(name, SODL_Value::Type::Coordinate, &data);
}

template<>
inline void SODL_Loader::addDataBinding<std::string>(const char* name, std::string &data) {
	addDataBindingImpl(name, SODL_Value::Type::String, &data);
}

template<>
inline void SODL_Loader::addDataBinding<bool>(const char* name, bool &data) {
	addDataBindingImpl(name, SODL_Value::Type::Bool, &data);
}

template<class C>
inline void SODL_Loader::addDataBinding(const char* name, C &data) {
	static_assert(std::is_convertible<C, void>(), "Data binding type not allowed");
}
