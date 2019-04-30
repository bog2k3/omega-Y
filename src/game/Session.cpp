#include "Session.h"

#include "../entities/FreeCamera.h"
#include "../entities/PlayerEntity.h"
#include "../terrain/Terrain.h"
#include "../sky/SkyBox.h"

#include <boglfw/World.h>
#include <boglfw/entities/CameraController.h>
#include <boglfw/utils/assert.h>

Session::Session(SessionType type, GameConfig config)
	: type_(type), gameCfg_(config) {
}

Session::~Session() {
}

Progress Session::load(unsigned step) {
	/*
	initSky();
	initTerrain(*pRenderData);*/
	switch (step) {
	case 0: {
		terrain_ = std::make_shared<Terrain>(false);
		World::getInstance().takeOwnershipOf(terrain_);

		freeCam_ = std::make_shared<FreeCamera>(glm::vec3{2.f, 1.f, 2.f}, glm::vec3{-1.f, -0.5f, -1.f});
		World::getInstance().takeOwnershipOf(freeCam_);

		// camera controller (this one moves the render camera to the position of the target entity)
		cameraCtrl_ = std::make_shared<CameraController>(nullptr);
		World::getInstance().takeOwnershipOf(cameraCtrl_);
		cameraCtrl_->attachToEntity(freeCam_, {0.f, 0.f, 0.f});

		player_ = std::make_shared<PlayerEntity>(glm::vec3{0.f, gameCfg_.terrainConfig.maxElevation + 10, 0.f}, 0.f);
		World::getInstance().takeOwnershipOf(player_);

		skyBox_ = std::make_shared<SkyBox>();
		World::getInstance().takeOwnershipOf(skyBox_);
	} break;
	case 1: {
		skyBox_->load(gameCfg_.skyBoxPath);
	} break;
	case 2: {
		terrain_->generate(gameCfg_.terrainConfig);
	} break;
	case 3: {
		terrain_->finishGenerate();
	} break;
	}
	return {step+1, 4};
}

Progress Session::unload(unsigned step) {
	return {1, 1};
}

void Session::start() {
	onStart.trigger();
}

void Session::stop() {
	onEnd.trigger();
}
