#ifndef SESSION_SETUP_HOST_CONTROLLER_H
#define SESSION_SETUP_HOST_CONTROLLER_H

#include "StateController.h"

#include <memory>

class SessionSetupHostMenu;
class OffscreenRenderer;
class Terrain;
class TerrainConfig;

class SessionSetupHostCtrl : public StateController
{
public:
	SessionSetupHostCtrl(GameState &state);
	~SessionSetupHostCtrl() override;

	void update(float dt) override;

private:
	std::shared_ptr<SessionSetupHostMenu> menu_;

	OffscreenRenderer *terrainRenderer_ = nullptr;
	TerrainConfig* terrainConfig_ = nullptr;
	Terrain* terrain_ = nullptr;

	void updateTerrain();
};

#endif // SESSION_SETUP_HOST_CONTROLLER_H
