#include "ButtonSODLWrapper.h"

ButtonSODLWrapper::ButtonSODLWrapper()
	: button_(new Button("unset")) {

	definePrimaryProperty("text", SODL_Value::Type::String);
	definePrimaryProperty("onClick", SODL_Value::Type::Callback);
}
