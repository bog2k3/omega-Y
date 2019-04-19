#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include <boglfw/GUI/GuiContainerElement.h>

#include <memory>

class Label;

class LoadingScreen : public GuiContainerElement {
public:
	LoadingScreen(glm::vec2 viewportSize);
	~LoadingScreen() override;

	void setProgress(float progress);

private:
	float progress_ = 0;
	std::shared_ptr<Label> label_;
};

#endif // LOADINGSCREEN_H
