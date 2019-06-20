#include "LobbyCtrl.h"

#include "../GUI/LobbyMenu.h"
#include "../net/handshake.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>
#include <boglfw/net/udp.h>
//#include <boglfw/net/connection.h>

#include <mutex>
#include <vector>

struct LobbyCtrl::NetData {
	net::udpSocket discoverSocket_;
	std::mutex mtxPendingHosts_;
	std::vector<std::string> pendingHosts_;
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
	stopHostsDiscovery();
	World::getGlobal<GuiSystem>()->removeElement(menu_);
}

// [async method]
void LobbyCtrl::addPendingHost(std::string ip) {
	std::lock_guard<std::mutex> lk(pNetData_->mtxPendingHosts_);
	pNetData_->pendingHosts_.push_back(ip);
}

void LobbyCtrl::update(float dt) {

}

// [async method]
void LobbyCtrl::hostFoundCallback(const char msgBuf[], size_t length, std::string hostIp) {
	if (length == sizeof(magicAdvertiseMessage) && !strncmp(msgBuf, magicAdvertiseMessage, sizeof(magicAdvertiseMessage))) {
		LOGLN("found new host on network at address: " << hostIp);
		addPendingHost(hostIp);
	}
}

void LobbyCtrl::startHostsDiscovery() {
	pNetData_->discoverSocket_ = net::createMulticastReceiveSocket("0.0.0.0", advertiseMulticastAddress, advertiseUDPPort);
	searchNextHost();
}

void LobbyCtrl::searchNextHost() {
	static char buf[256];
	assertDbg(sizeof(buf) >= sizeof(magicAdvertiseMessage));
	net::readUDPAsync(pNetData_->discoverSocket_, buf, sizeof(buf), [this] (net::result res, size_t length, net::endpointInfo sender) {
		if (res != net::result::ok) {
			ERROR("Async UDP read failed: " << res.code << ", " << res.message);
		} else {
			hostFoundCallback(buf, length, sender.address);
		}
		searchNextHost();
	});
}

void LobbyCtrl::stopHostsDiscovery() {
	net::closeSocket(pNetData_->discoverSocket_);
}
