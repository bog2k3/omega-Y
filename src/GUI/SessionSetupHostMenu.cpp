#include "SessionSetupHostMenu.h"

#include <boglfw/GUI/controls/Button.h>
#include <boglfw/GUI/controls/Label.h>
#include <boglfw/GUI/controls/Picture.h>

static const float margin = 0.02f; // of screen size

SessionSetupHostMenu::SessionSetupHostMenu(glm::vec2 viewportSize)
	: GuiContainerElement(viewportSize * margin, viewportSize * (1 - 2*margin))
{
	glm::vec2 mySize = getSize();

	std::shared_ptr<Label> pTitle = std::make_shared<Label>(glm::vec2{mySize.x / 2 - 125, 85}, 50, "Host Game");
	addElement(pTitle);

	std::shared_ptr<Button> pBack = std::make_shared<Button>(glm::vec2{10, mySize.y - 60}, glm::vec2{200, 50}, "Back");
	pBack->onClick.add([this](auto) {
		onBack.trigger();
	});
	addElement(pBack);

	std::shared_ptr<Button> pRegenerate = std::make_shared<Button>(glm::vec2{25, 620}, glm::vec2{160, 30}, "New terrain");
	pRegenerate->onClick.add([this](auto) {
		onRegenerate.trigger();
	});
	addElement(pRegenerate);

	std::shared_ptr<Button> pWireframe = std::make_shared<Button>(glm::vec2{165, 620}, glm::vec2{160, 30}, "Toggle Wireframe");
	pWireframe->onClick.add([this](auto) {
		onToggleWireframe.trigger();
	});
	addElement(pWireframe);

	pTerrainPicture_ = std::make_shared<Picture>(glm::vec2{25, 100}, glm::vec2{700, 500});
	pTerrainPicture_->onStartDrag.forward(onTerrainStartDrag);
	pTerrainPicture_->onEndDrag.forward(onTerrainEndDrag);
	pTerrainPicture_->onDrag.forward(onTerrainDrag);
	pTerrainPicture_->onScroll.forward(onTerrainZoom);
	addElement(pTerrainPicture_);
}

void SessionSetupHostMenu::setRTTexture(int texId) {
	pTerrainPicture_->setPictureTexture(texId, false);
}

glm::vec2 SessionSetupHostMenu::terrainPictureSize() const {
	return pTerrainPicture_->getSize();
}
