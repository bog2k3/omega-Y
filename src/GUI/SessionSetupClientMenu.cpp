#include "SessionSetupClientMenu.h"

#include <boglfw/GUI/controls/Button.h>
#include <boglfw/GUI/controls/Label.h>
#include <boglfw/GUI/controls/Picture.h>
#include <boglfw/GUI/GridLayout.h>

SessionSetupClientMenu::SessionSetupClientMenu() {
	setSize({100, 100, FlexCoord::PERCENT});
	setClientArea({50, 50}, {50, 50});
	auto layout = std::make_shared<GridLayout>();
	useLayout(layout);

	//glm::vec2{mySize.x / 2 - 100, 200}
	std::shared_ptr<Label> pTitle = std::make_shared<Label>(50, "Join Game");
	addElement(pTitle);

	//glm::vec2{10, mySize.y - 60}, glm::vec2{200, 50},
	std::shared_ptr<Button> pBack = std::make_shared<Button>("Back");
	pBack->onClick.forward(onBack);
	addElement(pBack);

	//glm::vec2{100, 100}, glm::vec2{400, 300}
	pTerrainPicture_ = std::make_shared<Picture>();
	addElement(pTerrainPicture_);
}

void SessionSetupClientMenu::setRTTexture(int texId) {
	pTerrainPicture_->setPictureTexture(texId);
}
