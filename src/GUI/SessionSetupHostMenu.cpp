#include "SessionSetupHostMenu.h"

#include <boglfw/GUI/controls/Button.h>
#include <boglfw/GUI/controls/Label.h>
#include <boglfw/GUI/controls/Picture.h>

static const float margin = 0.02f; // of screen size

SessionSetupHostMenu::SessionSetupHostMenu(glm::vec2 viewportSize)
	: GuiContainerElement(viewportSize * margin, viewportSize * (1 - 2*margin))
{
	glm::vec2 mySize = getSize();

	std::shared_ptr<Label> pTitle = std::make_shared<Label>(glm::vec2{mySize.x / 2 - 100, 200}, 50, "Host Game");
	addElement(pTitle);

	std::shared_ptr<Button> pBack = std::make_shared<Button>(glm::vec2{10, mySize.y - 60}, glm::vec2{200, 50}, "Back");
	pBack->onClick.add([this](auto) {
		onBack.trigger();
	});
	addElement(pBack);

	pTerrainPicture_ = std::make_shared<Picture>(glm::vec2{100, 100}, glm::vec2{400, 300});
	addElement(pTerrainPicture_);
}

void SessionSetupHostMenu::setRTTexture(int texId) {
	pTerrainPicture_->setPictureTexture(texId, true);
}

glm::vec2 SessionSetupHostMenu::terrainPictureSize() const {
	return pTerrainPicture_->getSize();
}
