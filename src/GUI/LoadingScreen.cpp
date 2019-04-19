#include "LoadingScreen.h"

#include <boglfw/GUI/controls/Label.h>

#include <sstream>

LoadingScreen::LoadingScreen(glm::vec2 viewportSize)
	: GuiContainerElement({0, 0}, viewportSize)
{
	label_ = std::make_shared<Label>(glm::vec2{400, 200}, 30, "0%");
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
