#include "LobbyMenu.h"
#include "HostEntry.h"

#include <boglfw/GUI/controls/ScrollingContainer.h>
#include <boglfw/GUI/controls/Button.h>
#include <boglfw/GUI/controls/Label.h>

static const float margin = 0.02f; // of screen size

struct LobbyMenu::LobbyData {
	std::shared_ptr<ScrollingContainer> spList;
};

LobbyMenu::LobbyMenu(glm::vec2 viewportSize)
	: GuiContainerElement(viewportSize * margin, viewportSize * (1 - 2*margin))
	, pData_(new LobbyData())
{
	glm::vec2 mySize = getSize();

	std::shared_ptr<Label> pTitle = std::make_shared<Label>(glm::vec2{mySize.x / 2 - 125, 85}, 50, "Join Game");
	addElement(pTitle);

	std::shared_ptr<Button> pBack = std::make_shared<Button>(glm::vec2{10, mySize.y - 60}, glm::vec2{200, 50}, "Back");
	pBack->onClick.forward(onBack);
	addElement(pBack);

	pData_->spList = std::make_shared<ScrollingContainer>(glm::vec2{20, 160}, glm::vec2{mySize.x - 40, 400});
	addElement(pData_->spList);
}

LobbyMenu::~LobbyMenu() {
	delete pData_, pData_ = nullptr;
}

void LobbyMenu::addHost(std::string ip) {
	auto spEntry = std::make_shared<HostEntry>(glm::vec2{10, 10}, glm::vec2{40, 380}, ip, ip);
	pData_->spList->addElement(spEntry);
}
