#include "ContainerSODLWrapper.h"

ContainerSODLWrapper::ContainerSODLWrapper() 
	: container_(new GuiContainerElement()) {
	defineSecondaryProperty("size", SODL_Property_Descriptor("coord2"));
	defineSecondaryProperty("padding", SODL_Property_Descriptor("coord4"));
	defineSecondaryProperty("layout", SODL_Property_Descriptor({
		{"free", "freeLayout"}, 
		{"fill", "fillLayout"}, 
		{"grid", "gridLayout"},
		{"split", "splitLayout"}
	}));
}