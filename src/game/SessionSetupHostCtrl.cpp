#include "SessionSetupHostCtrl.h"

#include "../GUI/SessionSetupHostMenu.h"
#include "../render/CustomRenderContext.h"

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

	auto terrainPictureSize = menu_->terrainPictureSize();
	FrameBufferDescriptor fbDesc;
	fbDesc.format = GL_RGB;
	fbDesc.width = terrainPictureSize.x;
	fbDesc.height = terrainPictureSize.y;
	auto renderCtx = new CustomRenderContext();
	renderCtx->renderPass = RenderPass::Standard;
	terrainRenderer_ = new OffscreenRenderer(fbDesc, std::unique_ptr<CustomRenderContext>(renderCtx));
	terrainRenderer_->viewport().camera().setFOV(PI/5.f);
	terrainRenderer_->viewport().camera().setZPlanes(0.15f, 500.f);
	terrainRenderer_->viewport().camera().moveTo({100, 70, 140});
	terrainRenderer_->viewport().camera().lookAt({0, 0, 0});

	menu_->setRTTexture(terrainRenderer_->getFBTexture());

	terrain_ = new Terrain(renderCtx->unifCommon);
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
	CustomRenderContext::fromCtx(terrainRenderer_->getRenderContext()).updateCommonUniforms();
	// prepare terrain picture
	terrainRenderer_->begin();
	terrainRenderer_->clear();
	// draw the terrain here ...
	terrainRenderer_->render(terrain_);
	terrainRenderer_->end();
}
