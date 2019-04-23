#include "LoadingCtrl.h"

#include "../GUI/LoadingScreen.h"
#include "../terrain/Terrain.h"
#include "../terrain/Water.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>

#include <map>

enum TaskNames {
	TerrainShaders,
	TerrainTextures,
	WaterShaders,
	WaterTextures,
};

static std::map<int, Progress (*)(unsigned)> tasksMap {
	{ TerrainShaders, Terrain::loadShaders },
	{ TerrainTextures, Terrain::loadTextures },
	{ WaterShaders, Water::loadShaders },
	{ WaterTextures, Water::loadTextures },
};

LoadingCtrl::LoadingCtrl(GameState &state, Situation situation)
	: StateController(state)
{
	switch (situation) {
	case INITIAL:
		tasks_.push_back(TerrainShaders);
		tasks_.push_back(TerrainTextures);
		tasks_.push_back(WaterShaders);
		tasks_.push_back(WaterTextures);

		nextState_ = GameState::StateNames::MAIN_MENU;
	break;
	case SESSION_START:
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
