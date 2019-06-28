#include "LoadingScreen.h"

#include <boglfw/GUI/controls/Label.h>

#include <sstream>

LoadingScreen::LoadingScreen() {
	setSize({100, 100, FlexCoord::PERCENT});
	label_ = std::make_shared<Label>(30, "0%");
	label_->setPosition({50, 50, FlexCoord::PERCENT});
	label_->setColor({1.f, 1.f, 1.f});
	addElement(label_);
}

LoadingScreen::~LoadingScreen()
{
}

void LoadingScreen::setProgress(float progress) {
	std::stringstream ss;
	ss << "Loading... " << (int)(progress * 100) << "%";
	label_->setText(ss.str());
}
