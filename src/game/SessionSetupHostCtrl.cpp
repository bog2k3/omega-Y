#include "SessionSetupHostCtrl.h"

#include "../GUI/SessionSetupHostMenu.h"
#include "../render/OffscreenRenderer.h"
#include "../render/CustomRenderContext.h"

#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>

#include <boglfw/renderOpenGL/Shape2D.h>

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
	terrainRenderer_ = new OffscreenRenderer(fbDesc, std::make_unique<CustomRenderContext>());

	menu_->setRTTexture(terrainRenderer_->getFBTexture());
}

SessionSetupHostCtrl::~SessionSetupHostCtrl() {
	World::getGlobal<GuiSystem>()->removeElement(menu_);
}

void SessionSetupHostCtrl::update(float dt) {
	// prepare terrain picture
	terrainRenderer_->begin();
	terrainRenderer_->clear();
	// draw the terrain here ...
	auto drawTest = [](RenderContext const&) {
		Shape2D::get()->drawCircleFilled({200, 100}, 90, 16, glm::vec3{1.f, 0.2, 0.4});
	};
	terrainRenderer_->render(&drawTest);
	terrainRenderer_->end();
}
