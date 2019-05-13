#include "NetHost.h"
#include "ConnectionWrapper.h"

#include <boglfw/net/listener.h>
#include <boglfw/net/connection.h>
#include <boglfw/utils/log.h>

#include <vector>

struct NetHost::HostData {
	net::listener listener;
	std::vector<std::shared_ptr<ConnectionWrapper>> connections;
};

static void onNewConnection(NetHost* pHost, net::result result, net::connection connection) {
	if (result.code != net::result::ok) {
		return;
	}
	pHost->addConnection(std::make_shared<ConnectionWrapper>(connection));
}

NetHost::NetHost(unsigned portNumber)
	: data_(new HostData()) {
	data_->listener = net::startListen(portNumber, std::bind(&onNewConnection, this, std::placeholders::_1, std::placeholders::_2));
}

NetHost::~NetHost() {
	net::stopListen(data_->listener);
	delete data_, data_ = nullptr;
}

void NetHost::addConnection(std::shared_ptr<ConnectionWrapper> con) {
	LOGLN("New client connected.");
	data_->connections.push_back(con);
}
