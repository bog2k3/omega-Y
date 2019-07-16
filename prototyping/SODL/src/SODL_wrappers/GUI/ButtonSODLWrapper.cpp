#include "ButtonSODLWrapper.h"

ButtonSODLWrapper::ButtonSODLWrapper()
	: button_(new Button("unset")) {

	definePrimaryProperty("text", SODL_Value::Type::String, nullptr);
	definePrimaryProperty("onClick", SODL_Value::Type::Callback, nullptr);
}
