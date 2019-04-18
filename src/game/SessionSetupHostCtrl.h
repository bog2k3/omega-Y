#ifndef SESSION_SETUP_HOST_CONTROLLER_H
#define SESSION_SETUP_HOST_CONTROLLER_H

#include "StateController.h"

#include <memory>

class SessionSetupHostMenu;
class OffscreenRenderer;

class SessionSetupHostCtrl : public StateController
{
public:
	SessionSetupHostCtrl(GameState &state);
	~SessionSetupHostCtrl() override;

	void draw(RenderContext const& ctx) override;

private:
	std::shared_ptr<SessionSetupHostMenu> menu_;

	OffscreenRenderer *terrainRenderer_ = nullptr;
};

#endif // SESSION_SETUP_HOST_CONTROLLER_H
