#pragma once

#include "GameConfig.h"
#include "../progress.h"

#include <boglfw/utils/Event.h>

#include <memory>

class Terrain;
class Water;
class FreeCamera;
class PlayerEntity;
class CameraController;
class SkyBox;

class Game {
public:
	Game(GameConfig cfg);
	~Game();

	Progress load(unsigned step);
	Progress unload(unsigned step);

	void start();
	void stop();
	bool isStarted() const { return started_; }

	std::shared_ptr<Terrain> terrain() const { return terrain_; }
	//std::shared_ptr<Water> water() const { return water_; }
	std::shared_ptr<SkyBox> skyBox() const { return skyBox_; }
	std::shared_ptr<PlayerEntity> player() const { return player_; }
	std::shared_ptr<FreeCamera> freeCam() const { return freeCam_; }
	std::shared_ptr<CameraController> cameraCtrl() const { return cameraCtrl_; }

	Event<void()> onStart;
	Event<void()> onEnd;

private:
	GameConfig config_;
	bool started_ = false;
	std::shared_ptr<Terrain> terrain_;
	//std::shared_ptr<Water> water_;
	std::shared_ptr<SkyBox> skyBox_;
	std::shared_ptr<PlayerEntity> player_;
	std::shared_ptr<FreeCamera> freeCam_;
	std::shared_ptr<CameraController> cameraCtrl_;
};
