#ifndef NET_ADAPTER_H
#define NET_ADAPTER_H

/* [NetAdapter]
	* both [NetHost] and [NetClient] objects are accessed via a [NetAdapter] interface, thus the rest of the code does not care about host/client identity.
	* this interface performs the relevant actions on network peers depending on wether it's a host or client type
	* The game code is isolated from network classes except for this interface.
	* implements the higher level communication protocol
 */

class NetAdapter {
public:
	virtual ~NetAdapter() {}
};

#endif //NET_ADAPTER_H
