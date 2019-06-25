#include "LobbyMenu.h"
#include "HostEntry.h"

#include <boglfw/GUI/controls/ScrollingContainer.h>
#include <boglfw/GUI/controls/Button.h>
#include <boglfw/GUI/controls/Label.h>

struct LobbyMenu::LobbyData {
	std::shared_ptr<ScrollingContainer> spList;
};

LobbyMenu::LobbyMenu(glm::vec2 viewportSize)
	: pData_(new LobbyData())
{
	glm::vec2 mySize = getSize();

	std::shared_ptr<Label> pTitle = std::make_shared<Label>(50, "Join Game");
	addElement(pTitle);

	//glm::vec2{10, mySize.y - 60}, glm::vec2{200, 50},
	std::shared_ptr<Button> pBack = std::make_shared<Button>("Back");
	pBack->onClick.forward(onBack);
	addElement(pBack);

	//glm::vec2{20, 160}, glm::vec2{mySize.x - 40, 400}
	pData_->spList = std::make_shared<ScrollingContainer>();
	addElement(pData_->spList);
}

LobbyMenu::~LobbyMenu() {
	delete pData_, pData_ = nullptr;
}

void LobbyMenu::addHost(std::string ip) {
	//glm::vec2{10, 10}, glm::vec2{40, 380},
	auto spEntry = std::make_shared<HostEntry>(ip, ip);
	spEntry->onClick.add([this, ip] {
		onJoinHost.trigger(ip);
	});
	pData_->spList->addElement(spEntry);
}
