#include "LobbyCtrl.h"

#include "../GUI/LobbyMenu.h"
#include "../net/handshake.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>
#include <boglfw/net/udp.h>
//#include <boglfw/net/connection.h>

#include <mutex>
#include <vector>

struct HostInfo {
	unsigned index;
	std::string ipAddress;
	bool isValid;
};

struct LobbyCtrl::NetData {
	net::udpSocket discoverSocket_;
	std::mutex mtxPendingHosts_;
	std::vector<std::string> pendingHosts_;
	std::map<std::string, HostInfo> discoveredHosts_;
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
	menu_->onJoinHost.add(std::bind(&LobbyCtrl::joinHost, this, std::placeholders::_1));

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
	// check if we found any new hosts on the network and add them to the list:
	std::vector<std::string> newHosts;
	{
		std::lock_guard<std::mutex> lk(pNetData_->mtxPendingHosts_);
		newHosts.swap(pNetData_->pendingHosts_);
	}
	for (auto &ip : newHosts) {
		if (pNetData_->discoveredHosts_.find(ip) == pNetData_->discoveredHosts_.end()) {
			// this host does not yet exist in the list, add it:
			LOGLN("found new host on network at address: " << ip);
			pNetData_->discoveredHosts_[ip] = {
				pNetData_->discoveredHosts_.size(),
				ip,
				false
			};
			getHostInfo(ip);
		}
	}
	// done checking for new hosts.
}

void LobbyCtrl::getHostInfo(std::string ip) {
	// connect to host and get some info, also negociate protocol
	// ...
	menu_->addHost(ip);
}

// [async method]
void LobbyCtrl::hostFoundCallback(const char msgBuf[], size_t length, std::string hostIp) {
	if (length == sizeof(magicAdvertiseMessage) && !strncmp(msgBuf, magicAdvertiseMessage, sizeof(magicAdvertiseMessage))) {
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

void LobbyCtrl::joinHost(std::string ip) {
	LOGLN("join host: " << ip);
}
