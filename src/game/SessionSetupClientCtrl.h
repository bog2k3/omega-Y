#ifndef SESSION_SETUP_CLIENT_CONTROLLER_H
#define SESSION_SETUP_CLIENT_CONTROLLER_H

#include "StateController.h"

#include <memory>

class SessionSetupClientMenu;

class SessionSetupClientCtrl : public StateController
{
public:
	SessionSetupClientCtrl(GameState &state);
	~SessionSetupClientCtrl() override;

	void draw(RenderContext const& ctx) override;

private:
	std::shared_ptr<SessionSetupClientMenu> menu_;
};

#endif // SESSION_SETUP_CLIENT_CONTROLLER_H
