#pragma once

#include "GuiElementSODLWrapper.h"

#include <boglfw/GUI/controls/Slider.h>

class SliderSODLWrapper : public GuiElementSODLWrapper {
public:
	std::string objectType() const override { return "slider"; }
	std::string superType() const override { return GuiElementSODLWrapper::objectType(); }

	~SliderSODLWrapper() override {}
	SliderSODLWrapper();

	std::shared_ptr<Slider> get() const { return slider_; }

protected:
	std::shared_ptr<ISODL_Object> clone() override;

private:
	std::shared_ptr<Slider> slider_;
	std::string label_;
	float rangeMin_;
	float rangeMax_;
	float step_;
	float value_;
	float precision_;
	float divisionStep_;
	float labelStep_;

	void onLoadingFinished();
};
