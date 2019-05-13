#include "SessionSetupHostCtrl.h"

#include "Session.h"
#include "SessionConfig.h"

#include "../GUI/SessionSetupHostMenu.h"
#include "../render/CustomRenderContext.h"
#include "../render/programs/UPackCommon.h"

#include "../terrain/Terrain.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/renderOpenGL/OffscreenRenderer.h>
#include <boglfw/utils/Timer.h>
#include <boglfw/utils/rand.h>

SessionSetupHostCtrl::SessionSetupHostCtrl(GameState &s)
	: StateController(s)
{
	SessionConfig sessionCfg;
	sessionCfg.type = SessionType::HOST;
	state_.initSession(sessionCfg);

	state_.session()->gameConfig().terrainConfig.seed = new_RID();
	state_.session()->gameConfig().terrainConfig.vertexDensity = 0.5;

	auto guiSystem = World::getGlobal<GuiSystem>();
	menu_ = std::make_shared<SessionSetupHostMenu>(guiSystem->getViewportSize(), &state_.session()->gameConfig().terrainConfig);
	guiSystem->addElement(menu_);

	menu_->onParametersChanged.add(std::bind(&SessionSetupHostCtrl::terrainConfigChanged, this));

	menu_->onBack.add([this]() {
		state_.destroySession();
		onNewStateRequest.trigger(GameState::StateNames::MAIN_MENU);
	});
	menu_->onStart.add([this]() {
		state_.session()->gameConfig().terrainConfig.vertexDensity = 1.f;
		state_.session()->initializeGame();
		onNewStateRequest.trigger(GameState::StateNames::SESSION_LOADING);
	});
	menu_->onTerrainStartDrag.add(std::bind(&SessionSetupHostCtrl::terrain_startDrag, this, std::placeholders::_1, std::placeholders::_2));
	menu_->onTerrainEndDrag.add(std::bind(&SessionSetupHostCtrl::terrain_endDrag, this));
	menu_->onTerrainDrag.add(std::bind(&SessionSetupHostCtrl::terrain_drag, this, std::placeholders::_1, std::placeholders::_2));
	menu_->onTerrainZoom.add(std::bind(&SessionSetupHostCtrl::terrain_zoom, this, std::placeholders::_1));

	auto terrainPictureSize = menu_->terrainPictureSize();
	FrameBufferDescriptor fbDesc;
	fbDesc.format = GL_RGB;
	fbDesc.width = terrainPictureSize.x;
	fbDesc.height = terrainPictureSize.y;
	fbDesc.requireDepthBuffer = true;
	auto renderCtx = new CustomRenderContext();
	renderCtx->renderPass = RenderPass::Standard;
	terrainRenderer_ = new OffscreenRenderer(fbDesc, std::unique_ptr<CustomRenderContext>(renderCtx));
	terrainRenderer_->viewport().camera().setFOV(PI/5.f);
	terrainRenderer_->viewport().camera().setZPlanes(0.15f, 500.f);
	terrainRenderer_->viewport().camera().moveTo({140, 50, 180});
	terrainRenderer_->viewport().camera().lookAt({0, 0, 0});

	menu_->setRTTexture(terrainRenderer_->getFBTexture());

	terrain_ = new Terrain(true);
	updateTerrain();

	terrainTimer_ = new Timer(0.5f);
	terrainTimer_->onTimeout.add(std::bind(&SessionSetupHostCtrl::updateTerrain, this));
}

SessionSetupHostCtrl::~SessionSetupHostCtrl() {
	World::getGlobal<GuiSystem>()->removeElement(menu_);
	delete terrain_;
	delete terrainTimer_;
}

void SessionSetupHostCtrl::terrainConfigChanged() {
	terrainTimer_->restart();
}

void SessionSetupHostCtrl::updateTerrain() {
	terrain_->generate(state_.session()->gameConfig().terrainConfig);
	terrain_->finishGenerate();
}

void SessionSetupHostCtrl::update(float dt) {
	terrainTimer_->update(dt);

	CustomRenderContext &rctx = CustomRenderContext::fromCtx(terrainRenderer_->getRenderContext());
	rctx.updateCommonUniforms();
	// prepare terrain picture
	terrainRenderer_->begin();
	terrainRenderer_->clear();
	// draw the terrain here ...
	terrainRenderer_->render(terrain_);
	terrainRenderer_->end();
}

void SessionSetupHostCtrl::terrain_startDrag(float x, float y) {
	//terrain_->setWireframeMode(true);
}

void SessionSetupHostCtrl::terrain_endDrag() {
	//terrain_->setWireframeMode(false);
}
void SessionSetupHostCtrl::terrain_drag(float dx, float dy) {
	float rotateSpeed = 0.01f; // rad / pixels of movement
	glm::vec3 up = terrainRenderer_->viewport().camera().localY();
	glm::vec3 right = terrainRenderer_->viewport().camera().localX();
	glm::quat qX = glm::angleAxis(dx * rotateSpeed, up);
	glm::quat qY = glm::angleAxis(dy * rotateSpeed, right);
	terrainRenderer_->viewport().camera().orbit({0.f, 0.f, 0.f}, qX*qY, true);
}

void SessionSetupHostCtrl::terrain_zoom(float dz) {
	const float zoomStep = 1.1f;
	float zoomFactor = dz < 0 ? zoomStep : 1.f / zoomStep;
	const float minFOV = PI / 20;
	const float maxFOV = PI / 4;
	float newFOV = clamp(terrainRenderer_->viewport().camera().FOV() * zoomFactor, minFOV, maxFOV);
	terrainRenderer_->viewport().camera().setFOV(newFOV);
}
