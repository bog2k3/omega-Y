#include "ContainerSODLWrapper.h"

ContainerSODLWrapper::ContainerSODLWrapper() 
	: container_(new GuiContainerElement()) {
	defineSecondaryProperty("size", SODL_Property_Descriptor("coord2", nullptr));
	defineSecondaryProperty("padding", SODL_Property_Descriptor("coord4", nullptr));
	defineSecondaryProperty("layout", SODL_Property_Descriptor({
		{"free", "freeLayout"}, 
		{"fill", "fillLayout"}, 
		{"grid", "gridLayout"},
		{"split", "splitLayout"}
	}, nullptr));
}
