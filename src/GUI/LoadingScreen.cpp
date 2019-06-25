#include "LoadingScreen.h"

#include <boglfw/GUI/controls/Label.h>

#include <sstream>

LoadingScreen::LoadingScreen() {
	//glm::vec2{400, 200},
	label_ = std::make_shared<Label>(30, "0%");
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
