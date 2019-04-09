#ifndef SESSION_H
#define SESSION_H

#include <boglfw/renderOpenGL/drawable.h>
#include <boglfw/utils/updatable.h>

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
		LOBBY,		// main menu before a game session is established
		HOST_SETUP,	// host game setup screen
		JOIN_WAIT,	// after joining a session that is being setup by the host
		GAME,		// during game play, after the session is started by the host
	};

	Session(SessionType type);
	~Session();

	SessionType type() const { return type_; }

	std::vector<drawable> & drawList3D() { return drawList3D_; }
	std::vector<drawable> & drawList2D() { return drawList2D_; }

	std::weak_ptr<FreeCamera> freeCam() const { return freeCam_; }
	std::weak_ptr<PlayerEntity> player() const { return player_; }
	std::weak_ptr<CameraController> cameraCtrl() const { return cameraCtrl_; }

	void update(float dt);

	bool enableWaterRender_ = false;
	Terrain* pTerrain_ = nullptr;
	Water* pWater_ = nullptr;

private:
	SessionType type_;
	std::vector<drawable> drawList3D_;
	std::vector<drawable> drawList2D_;

	std::weak_ptr<FreeCamera> freeCam_;
	std::weak_ptr<PlayerEntity> player_;
	std::weak_ptr<CameraController> cameraCtrl_;

	friend Session* createLobbySession();
	friend Session* createHostSession();
	friend Session* createJoinSession();
	friend Session* createGameSession();
};

Session* createLobbySession();
Session* createHostSession();
Session* createJoinSession();
Session* createGameSession();

#endif // SESSION_H
