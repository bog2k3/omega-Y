#include "NetClient.h"
#include "ConnectionWrapper.h"

#include <boglfw/net/connection.h>
#include <boglfw/utils/log.h>

struct NetClient::ClientData {
	std::shared_ptr<ConnectionWrapper> connection;
};

static void onConnected(NetClient* pClient, net::result result, net::connection connection) {
	if (result.code != net::result::ok)
		pClient->setConnection(nullptr);
	else
		pClient->setConnection(std::make_shared<ConnectionWrapper>(connection));
}

NetClient::NetClient(std::string hostUrl, unsigned portNumber)
	: data_(new ClientData()) {
	net::connect_async(hostUrl.c_str(), portNumber, std::bind(&onConnected, this, std::placeholders::_1, std::placeholders::_2));
}

NetClient::~NetClient() {
	delete data_, data_ = nullptr;
}

void NetClient::setConnection(std::shared_ptr<ConnectionWrapper> con) {
	LOGLN("Connected to host.");
	data_->connection = con;
}
