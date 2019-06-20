#include "HostEntry.h"
#include "../ResourceManager.h"

#include <boglfw/GUI/controls/Picture.h>

HostEntry::HostEntry(glm::vec2 pos, glm::vec2 size, std::string hostName, std::string ip)
	: GuiContainerElement(pos, size)
	, hostName_(hostName)
	, ip_(ip)
{
	auto spLogo = std::make_shared<Picture>(pos + glm::vec2{10, 10}, glm::vec2{64, 64});
	spLogo->setPictureTexture(ResourceManager::getTexture("data/logo-small.png"));
	addElement(spLogo);
}

HostEntry::~HostEntry() {

}
