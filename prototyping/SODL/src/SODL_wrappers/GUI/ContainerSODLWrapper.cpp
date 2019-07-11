#include "ContainerSODLWrapper.h"

ContainerSODLWrapper::ContainerSODLWrapper() 
	: container_(new GuiContainerElement()) {
	defineSecondaryProperty("size", SODL_Property_Descriptor("vec2"));
	defineSecondaryProperty("padding", SODL_Property_Descriptor("vec4"));
	defineSecondaryProperty("layout", SODL_Property_Descriptor("layout"));
}