#ifndef NET_CLIENT_H
#define NET_CLIENT_H

#include "NetAdapter.h"
#include "ConnectionWrapper.h"

#include <string>
#include <memory>

/* [NetClient class]
	* imlements NetAdapter interface
	* holds a single ConnectionWrapper for the connection with the host
	* sends local game events to the host
	* receives remote game events from the host
*/

class NetClient : public NetAdapter {
public:
	NetClient(std::string hostUrl, unsigned portNumber);
	~NetClient() override;

	void setConnection(std::shared_ptr<ConnectionWrapper> con);

private:
	struct ClientData;
	ClientData* data_;
};

#endif //NET_CLIENT_H
