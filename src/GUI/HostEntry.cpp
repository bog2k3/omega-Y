#include "HostEntry.h"
#include "../ResourceManager.h"

#include <boglfw/GUI/controls/Picture.h>
#include <boglfw/GUI/controls/Label.h>

HostEntry::HostEntry(glm::vec2 pos, glm::vec2 size, std::string hostName, std::string ip)
	: GuiContainerElement(pos, size)
	, hostName_(hostName)
	, ip_(ip)
{
	auto spLogo = std::make_shared<Picture>(pos + glm::vec2{10, 10}, glm::vec2{64, 64});
	spLogo->setPictureTexture(ResourceManager::getTexture("data/logo-small.png"));
	addElement(spLogo);

	auto spHostName = std::make_shared<Label>(pos + glm::vec2{200, 10}, 16, hostName);
	addElement(spHostName);

	auto spHostIp = std::make_shared<Label>(pos + glm::vec2{400, 10}, 16, ip);
	addElement(spHostIp);
}

HostEntry::~HostEntry() {

}
