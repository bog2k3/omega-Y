#include "Session.h"

#include "entities/FreeCamera.h"
#include "entities/PlayerEntity.h"

#include <boglfw/World.h>
#include <boglfw/entities/CameraController.h>
#include <boglfw/utils/assert.h>

Session::Session(SessionType type)
	: type_(type) {
}

Session::~Session() {
}

Progress Session::load(unsigned step) {
	/*
	initSky();
	initTerrain(*pRenderData);*/
	switch (step) {
	case 0:
		auto sTerrain = std::make_shared<Terrain>(false);
		terrain_ = sTerrain;
		World::getInstance().takeOwnershipOf(sTerrain);

		auto sFreeCam = std::make_shared<FreeCamera>(glm::vec3{2.f, 1.f, 2.f}, glm::vec3{-1.f, -0.5f, -1.f});
		freeCam_ = sFreeCam;
		World::getInstance().takeOwnershipOf(sFreeCam);

		// camera controller (this one moves the render camera to the position of the target entity)
		auto sCamCtrl = std::make_shared<CameraController>(&pRenderData->viewport.camera());
		cameraCtrl = sCamCtrl;
		World::getInstance().takeOwnershipOf(sCamCtrl);
		sCamCtrl->attachToEntity(freeCam_, {0.f, 0.f, 0.f});

		auto sPlayer = std::make_shared<PlayerEntity>(glm::vec3{0.f, gameCfg_.terrainConfig.maxElevation + 10, 0.f}, 0.f);
		player_ = sPlayer;
		World::getInstance().takeOwnershipOf(sPlayer);

		pSkyBox = new SkyBox();
		// pSkyBox->load("data/textures/sky/1");
	break;
	case 1:
		GameState::session()->pTerrain->generate(GameState::session()->gameCfg.terrainConfig);
	break;
	case 2:
		GameState::session()->pTerrain->finishGenerate();
	break;
	}
	return {step+1, 3};
}

Progress Session::unload(unsigned step) {
	return {1, 1};
}
