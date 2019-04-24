#include "LoadingCtrl.h"

#include "../GUI/LoadingScreen.h"
#include "../terrain/Terrain.h"
#include "../terrain/Water.h"

#include "../render/ShaderProgramManager.h"
#include "../render/programs/ShaderTerrain.h"
#include "../render/programs/ShaderTerrainPreview.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>

#include <map>

class ShaderTerrainPreview;
class ShaderTerrain;
//class ShaderWater;

enum TaskNames {
	InitialShaders,
	TerrainTextures,
	WaterTextures,
	SessionShaders,
};

Progress loadInitialShaders(unsigned step) {
	ShaderProgramManager::requestProgram<ShaderTerrainPreview>();

	return {1, 1};
}

Progress loadSessionShaders(unsigned step) {
	ShaderProgramManager::requestProgram<ShaderTerrain>();
	//ShaderProgramManager::requestProgram<ShaderWater>();

	return {1, 1};
}

static std::map<int, Progress (*)(unsigned)> tasksMap {
	{ InitialShaders, loadInitialShaders },
	{ SessionShaders, loadSessionShaders },
	{ TerrainTextures, Terrain::loadTextures },
	{ WaterTextures, Water::loadTextures },
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

		nextState_ = GameState::StateNames::GAMEPLAY;
	break;
	case SESSION_END:
		nextState_ = GameState::StateNames::MAIN_MENU;
	break;
	}

	auto guiSystem = World::getGlobal<GuiSystem>();
	loadingScreen_ = std::make_shared<LoadingScreen>(guiSystem->getViewportSize());
	guiSystem->addElement(loadingScreen_);
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
