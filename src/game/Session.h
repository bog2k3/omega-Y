#ifndef SESSION_H
#define SESSION_H

#include "GameConfig.h"
#include "../progress.h"

//#include <boglfw/renderOpenGL/drawable.h>
//#include <boglfw/utils/updatable.h>
#include <boglfw/utils/Event.h>


#include <vector>
#include <memory>

class Terrain;
class Water;
class FreeCamera;
class PlayerEntity;
class CameraController;
class SkyBox;

class Session {
public:
	enum SessionType {
		SESSION_HOST,
		SESSION_CLIENT,
	};

	Session(SessionType type, GameConfig config);
	~Session();

	SessionType type() const { return type_; }
	GameConfig const& gameConfig() { return gameCfg_; }

	std::shared_ptr<Terrain> terrain() const { return terrain_.lock(); }
	std::shared_ptr<Water> water() const { return water_.lock(); }
	std::shared_ptr<SkyBox> skyBox() const { return skyBox_.lock(); }
	std::shared_ptr<PlayerEntity> player() const { return player_.lock(); }
	std::shared_ptr<FreeCamera> freeCam() const { return freeCam_.lock(); }
	std::shared_ptr<CameraController> cameraCtrl() const { return cameraCtrl_.lock(); }

	//std::vector<drawable> & drawList3D() { return drawList3D_; }
	//std::vector<drawable> & drawList2D() { return drawList2D_; }

	//void update(float dt);

	Progress load(unsigned step);
	Progress unload(unsigned step);

	Event<void()> onStart;
	Event<void()> onEnd;

private:
	SessionType type_;
	GameConfig gameCfg_;
	std::weak_ptr<Terrain> terrain_;
	std::weak_ptr<Water> water_;
	std::weak_ptr<SkyBox> skyBox_;
	std::weak_ptr<PlayerEntity> player_;
	std::weak_ptr<FreeCamera> freeCam_;
	std::weak_ptr<CameraController> cameraCtrl_;

	//std::vector<drawable> drawList3D_;
	//std::vector<drawable> drawList2D_;

};

#endif // SESSION_H
