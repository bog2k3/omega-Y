#ifndef LOBBY_CONTROLLER_H
#define LOBBY_CONTROLLER_H

#include "StateController.h"

#include <memory>
#include <string>

class LobbyMenu;

class LobbyCtrl : public StateController
{
public:
	LobbyCtrl(GameState &state);
	~LobbyCtrl() override;

	void update(float dt) override;

private:
	std::shared_ptr<LobbyMenu> menu_;

	struct NetData;
	NetData *pNetData_;

	void startHostsDiscovery();
	void stopHostsDiscovery();
	void searchNextHost();
	// [async method]
	void hostFoundCallback(const char msgBuf[], size_t length, std::string hostIp);
	// [async method]
	void addPendingHost(std::string ip);
};

#endif // LOBBY_CONTROLLER_H
