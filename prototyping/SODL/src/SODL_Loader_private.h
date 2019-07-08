#pragma once

#include "SODL_loader.h"

#include <memory>
#include <cassert>

class _SODL_Loader_ActionBindingModel {
public:
	virtual ~_SODL_Loader_ActionBindingModel() {}
};

template<class FuncType>
class _SODL_Loader_ActionBindingWrapper : public _SODL_Loader_ActionBindingModel {
public:
	_SODL_Loader_ActionBindingWrapper(std::function<FuncType> func)
		: func_(func) {
	}
	~_SODL_Loader_ActionBindingWrapper() {}
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
