#include "HostEntry.h"
#include "../ResourceManager.h"

#include <boglfw/GUI/controls/Picture.h>
#include <boglfw/GUI/controls/Label.h>

HostEntry::HostEntry(std::string hostName, std::string ip)
	: hostName_(hostName)
	, ip_(ip)
{
	auto spLogo = std::make_shared<Picture>();
	spLogo->setSize({64, 64});
	spLogo->setPictureTexture(ResourceManager::getTexture("data/logo-small.png"));
	addElement(spLogo);

	auto spHostName = std::make_shared<Label>(16, hostName);
	addElement(spHostName);

	auto spHostIp = std::make_shared<Label>(16, ip);
	addElement(spHostIp);
}

HostEntry::~HostEntry() {

}

void HostEntry::clicked(glm::vec2 clickPosition, MouseButtons button) {
	if (button == MouseButtons::Left)
		onClick.trigger();
}
