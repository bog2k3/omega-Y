#include "LoadingCtrl.h"

#include "Session.h"
#include "Game.h"

#include "../GUI/LoadingScreen.h"
#include "../terrain/Terrain.h"
#include "../terrain/Water.h"

#include "../render/ShaderProgramManager.h"
#include "../render/programs/ShaderTerrain.h"
#include "../render/programs/ShaderTerrainPreview.h"
#include "../render/programs/ShaderWater.h"
#include "../render/programs/ShaderSkybox.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>

#include <map>

enum TaskNames {
	InitialShaders,
	TerrainTextures,
	WaterTextures,
	SessionShaders,
	GameLoad,
	GameUnload,
};

Progress loadInitialShaders(unsigned step) {
	ShaderProgramManager::requestProgram<ShaderTerrainPreview>();

	return {1, 1};
}

Progress loadSessionShaders(unsigned step) {
	ShaderProgramManager::requestProgram<ShaderTerrain>();
	ShaderProgramManager::requestProgram<ShaderWater>();
	ShaderProgramManager::requestProgram<ShaderSkybox>();

	return {1, 1};
}

Progress gameLoad(unsigned step) {
	return GameState::session()->game()->load(step);
}

Progress gameUnload(unsigned step) {
	return GameState::session()->game()->unload(step);
}

static std::map<int, progressiveFunction> tasksMap {
	{ InitialShaders, loadInitialShaders },
	{ SessionShaders, loadSessionShaders },
	{ TerrainTextures, Terrain::loadTextures },
	{ WaterTextures, Water::loadTextures },
	{ GameLoad, gameLoad },
	{ GameUnload, gameUnload },
};

LoadingCtrl::LoadingCtrl(GameState &state, Situation situation)
	: StateController(state)
{
	switch (situation) {
	case INITIAL:
		tasks_.push_back(InitialShaders);
		tasks_.push_back(TerrainTextures);

		nextState_ = GameState::StateNames::MAIN_MENU;
	break;
	case SESSION_START:
		tasks_.push_back(SessionShaders);
		tasks_.push_back(WaterTextures);
		tasks_.push_back(GameLoad);

		nextState_ = GameState::StateNames::GAMEPLAY;
	break;
	case SESSION_END:
		tasks_.push_back(GameUnload);

		nextState_ = GameState::StateNames::MAIN_MENU;
	break;
	}

	loadingScreen_ = std::make_shared<LoadingScreen>();
	World::getGlobal<GuiSystem>()->addElement(loadingScreen_);
}

LoadingCtrl::~LoadingCtrl() {
	World::getGlobal<GuiSystem>()->removeElement(loadingScreen_);
}

void LoadingCtrl::update(float dt) {
	if (taskPointer_ == tasks_.size()) {
		onNewStateRequest.trigger(nextState_);
		return;
	}
	Progress crtProgress = tasksMap[tasks_[taskPointer_]](taskProgress_);
	taskProgress_ = crtProgress.completed;
	if (taskProgress_ == crtProgress.total) {
		taskProgress_ = 0;
		crtProgress.completed = 0;
		taskPointer_++;
	}

	float totalProgress = (taskPointer_ + ((float)crtProgress.completed / crtProgress.total)) / tasks_.size();
	loadingScreen_->setProgress(totalProgress);
}
