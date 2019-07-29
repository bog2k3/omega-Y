#include "LobbyMenu.h"
#include "HostEntry.h"

#include "../sodl/SODL_loader.h"
#include "../sodl/GUI_SODL_OBJ_Factory.h"
#include "../sodl/wrappers/GUI/ContainerSODLWrapper.h"

#include <boglfw/GUI/controls/ScrollingContainer.h>
#include <boglfw/utils/log.h>

struct LobbyMenu::LobbyData {
	std::shared_ptr<ScrollingContainer> spList;
};

LobbyMenu::LobbyMenu(glm::vec2 viewportSize)
	: pData_(new LobbyData())
{
}

void LobbyMenu::load() {
	GUI_SODL_OBJ_Factory objFactory;
	SODL_Loader loader(objFactory);

	loader.addActionBinding<>("goBack", {}, onBack);

	ContainerSODLWrapper root(*this);
	auto res = loader.mergeObject(root, "data/ui/lobbyMenu.sodl");
	if (!res) {
		ERROR("Failed to load lobbyMenu SODL: " << res.toString());
		return;
	}
	pData_->spList = root.getElement<ScrollingContainer>("hostsList");
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
