#ifndef SESSION_H
#define SESSION_H

#include "GameConfig.h"
//#include <boglfw/renderOpenGL/drawable.h>
//#include <boglfw/utils/updatable.h>


#include <vector>
#include <memory>

class Terrain;
class Water;
class FreeCamera;
class PlayerEntity;
class CameraController;

class Session {
public:
	enum SessionType {
		SESSION_HOST,
		SESSION_CLIENT,
	};

	Session(SessionType type);
	~Session();

	SessionType type() const { return type_; }

	//std::vector<drawable> & drawList3D() { return drawList3D_; }
	//std::vector<drawable> & drawList2D() { return drawList2D_; }

	std::weak_ptr<FreeCamera> freeCam() const { return freeCam_; }
	std::weak_ptr<PlayerEntity> player() const { return player_; }
	std::weak_ptr<CameraController> cameraCtrl() const { return cameraCtrl_; }

	//void update(float dt);

	GameConfig gameCfg_;
	//bool enableWaterRender_ = false;
	Terrain* pTerrain_ = nullptr;
	Water* pWater_ = nullptr;

private:
	SessionType type_;
	//std::vector<drawable> drawList3D_;
	//std::vector<drawable> drawList2D_;

	std::weak_ptr<FreeCamera> freeCam_;
	std::weak_ptr<PlayerEntity> player_;
	std::weak_ptr<CameraController> cameraCtrl_;
};

#endif // SESSION_H
