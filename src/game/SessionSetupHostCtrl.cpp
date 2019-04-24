#include "SessionSetupHostCtrl.h"

#include "../GUI/SessionSetupHostMenu.h"
#include "../render/CustomRenderContext.h"
#include "../render/programs/UPackCommon.h"

#include "../terrain/Terrain.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/renderOpenGL/OffscreenRenderer.h>

SessionSetupHostCtrl::SessionSetupHostCtrl(GameState &s)
	: StateController(s)
{
	auto guiSystem = World::getGlobal<GuiSystem>();
	menu_ = std::make_shared<SessionSetupHostMenu>(guiSystem->getViewportSize());
	guiSystem->addElement(menu_);

	menu_->onBack.add([this]() {
		// delete session
		// ...
		onNewStateRequest.trigger(GameState::StateNames::MAIN_MENU);
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
	terrainRenderer_->viewport().camera().moveTo({100, 70, 140});
	terrainRenderer_->viewport().camera().lookAt({0, 0, 0});

	menu_->setRTTexture(terrainRenderer_->getFBTexture());

	terrain_ = new Terrain(true);
	terrainConfig_ = new TerrainConfig();
	terrainConfig_->length = 100;
	terrainConfig_->width = 100;
	terrainConfig_->vertexDensity = 1.0;
	updateTerrain();
}

SessionSetupHostCtrl::~SessionSetupHostCtrl() {
	World::getGlobal<GuiSystem>()->removeElement(menu_);
	delete terrain_;
	delete terrainConfig_;
}

void SessionSetupHostCtrl::updateTerrain() {
	terrain_->generate(*terrainConfig_);
	terrain_->finishGenerate();
}

void SessionSetupHostCtrl::update(float dt) {
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
	terrain_->setWireframeMode(true);
}

void SessionSetupHostCtrl::terrain_endDrag() {
	terrain_->setWireframeMode(false);
}
void SessionSetupHostCtrl::terrain_drag(float dx, float dy) {

}

void SessionSetupHostCtrl::terrain_zoom(float dz) {

}