#ifndef NET_HOST_H
#define NET_HOST_H

#include "NetAdapter.h"

/* [NetHost class]
	* imlements NetAdapter interface
	* holds a list of ConnectionWrappers that represent connected clients.
	* broadcasts local game events to all clients
	* receives game events from each client and broadcasts them to all other clients
*/

class NetHost : public NetAdapter {
public:
	~NetHost() override {}
};

#endif //NET_HOST_H
