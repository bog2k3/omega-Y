#include "ButtonSODLWrapper.h"

ButtonSODLWrapper::ButtonSODLWrapper()
	: button_(new Button("unset")) {

	definePrimaryProperty<std::string>("text", nullptr);
	definePrimaryProperty<void*>("onClick", nullptr);
}
