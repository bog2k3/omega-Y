#include "NetHost.h"
#include "ConnectionWrapper.h"
#include "handshake.h"

#include <boglfw/net/listener.h>
#include <boglfw/net/connection.h>
#include <boglfw/net/udp.h>
#include <boglfw/utils/log.h>

#include <vector>
#include <thread>
#include <atomic>

struct NetHost::HostData {
	net::listener listener;
	net::udpSocket advertiseSocket;
	std::vector<std::shared_ptr<ConnectionWrapper>> connections;
	std::thread thrAdvertise;
	std::atomic_bool stopAdvertise { false };
};

static void advertiseFunc(std::atomic_bool* pStopSignal) {
	// set-up multicast
	net::udpSocket advertiseSocket = net::createMulticastSendSocket(advertiseMulticastAddress, advertiseUDPPort);
	void* sendBuffer = (void*)magicAdvertiseMessage;
	unsigned sendBytes = sizeof(magicAdvertiseMessage);

	float pingInterval = 2.f; // seconds
	float sleepInterval = 0.5f; // seconds

	float partialTime = 0;
	while (!pStopSignal->load(std::memory_order_acquire)) {
		std::this_thread::sleep_for(std::chrono::milliseconds((int)(1000 * sleepInterval)));
		partialTime += sleepInterval;
		if (partialTime > pingInterval) {
			partialTime -= pingInterval;
			// send a multicast message to advertise the server's presence
			net::writeUDP(advertiseSocket, sendBuffer, sendBytes);
		}
	}
	// clean-up
	net::closeSocket(advertiseSocket);
}

static void onNewConnection(NetHost* pHost, net::result result, net::connection connection) {
	if (result.code != net::result::ok) {
		return;
	}
	pHost->addConnection(std::make_shared<ConnectionWrapper>(connection));
}

NetHost::NetHost(unsigned portNumber)
	: data_(new HostData()) {
	data_->listener = net::startListen(portNumber, std::bind(&onNewConnection, this, std::placeholders::_1, std::placeholders::_2));
	data_->thrAdvertise = std::thread(advertiseFunc, &data_->stopAdvertise);
}

NetHost::~NetHost() {
	net::stopListen(data_->listener);
	data_->stopAdvertise.store(true, std::memory_order_release);
	data_->thrAdvertise.join();
	delete data_, data_ = nullptr;
}

void NetHost::addConnection(std::shared_ptr<ConnectionWrapper> con) {
	LOGLN("New client connected.");
	data_->connections.push_back(con);
}
