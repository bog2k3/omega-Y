#include "LobbyCtrl.h"

#include "../GUI/LobbyMenu.h"
#include "../net/handshake.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>
#include <boglfw/net/udp.h>
//#include <boglfw/net/connection.h>

struct LobbyCtrl::NetData {
	net::udpSocket discoverSocket_;
};

LobbyCtrl::LobbyCtrl(GameState &s)
	: StateController(s)
	, pNetData_(new NetData())
{
	auto guiSystem = World::getGlobal<GuiSystem>();
	menu_ = std::make_shared<LobbyMenu>(guiSystem->getViewportSize());
	guiSystem->addElement(menu_);

	menu_->onBack.add([this]() {
		// delete session
		// ...
		onNewStateRequest.trigger(GameState::StateNames::MAIN_MENU);
	});

	startHostsDiscovery();
}

LobbyCtrl::~LobbyCtrl() {
	World::getGlobal<GuiSystem>()->removeElement(menu_);
}

void LobbyCtrl::startHostsDiscovery() {
	pNetData_->discoverSocket_ = net::createMulticastReceiveSocket("0.0.0.0", advertiseMulticastAddress, advertiseUDPPort);
	net::endpointInfo sender;
	char buf[256];
	size_t length;
	net::result res = net::readUDP(pNetData_->discoverSocket_, buf, sizeof(buf), length, sender);
	if (res == net::result::ok) {
		if (length == sizeof(magicAdvertiseMessage) && !strncmp(buf, magicAdvertiseMessage, sizeof(magicAdvertiseMessage))) {
			LOGLN("found new host on network at address: " << sender.address);
		}
	}
}

void LobbyCtrl::stopHostsDiscovery() {
	net::closeSocket(pNetData_->discoverSocket_);
}
