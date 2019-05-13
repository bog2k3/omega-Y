#ifndef NET_HOST_H
#define NET_HOST_H

#include "NetAdapter.h"
#include "ConnectionWrapper.h"

#include <memory>

/* [NetHost class]
	* imlements NetAdapter interface
	* holds a list of ConnectionWrappers that represent connected clients.
	* broadcasts local game events to all clients
	* receives game events from each client and broadcasts them to all other clients
*/

class NetHost : public NetAdapter {
public:
	explicit NetHost(unsigned portNumber);
	~NetHost() override;

	void addConnection(std::shared_ptr<ConnectionWrapper> con);

private:
	struct HostData;
	HostData* data_;
};

#endif //NET_HOST_H
