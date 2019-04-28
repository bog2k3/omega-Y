#ifndef SESSION_SETUP_HOST_CONTROLLER_H
#define SESSION_SETUP_HOST_CONTROLLER_H

#include "StateController.h"

#include <memory>

class SessionSetupHostMenu;
class OffscreenRenderer;
class Terrain;
class TerrainConfig;
class Timer;

class SessionSetupHostCtrl : public StateController
{
public:
	SessionSetupHostCtrl(GameState &state);
	~SessionSetupHostCtrl() override;

	void update(float dt) override;

private:
	std::shared_ptr<SessionSetupHostMenu> menu_;

	OffscreenRenderer *terrainRenderer_;
	Terrain* terrain_;
	Timer* terrainTimer_;

	void terrainConfigChanged();
	void updateTerrain();
	void terrain_startDrag(float x, float y);
	void terrain_endDrag();
	void terrain_drag(float dx, float dy);
	void terrain_zoom(float dz);
};

#endif // SESSION_SETUP_HOST_CONTROLLER_H
