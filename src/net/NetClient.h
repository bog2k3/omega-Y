#ifndef NET_CLIENT_H
#define NET_CLIENT_H

#include "NetAdapter.h"

/* [NetClient class]
	* imlements NetAdapter interface
	* holds a single ConnectionWrapper for the connection with the host
	* sends local game events to the host
	* receives remote game events from the host
*/

class NetClient : public NetAdapter {
public:
	~NetClient() override {}
};

#endif //NET_CLIENT_H
