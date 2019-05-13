#include "ConnectionWrapper.h"

ConnectionWrapper::ConnectionWrapper(net::connection con)
	: connection_(con) {
	// TODO begin negociation...
}

ConnectionWrapper::~ConnectionWrapper() {
	net::closeConnection(connection_);
}
