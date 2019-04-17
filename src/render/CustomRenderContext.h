#ifndef __CUST_RENDER_CONTEXT_H__
#define __CUST_RENDER_CONTEXT_H__

#include <boglfw/renderOpenGL/RenderContext.h>

#include <glm/vec4.hpp>

enum class RenderPass {
	None,
	WaterReflection,	// off-screen rendering for water reflection texture
	WaterRefraction,	// off-screen rendering for water refraction texture
	Standard,			// standard default rendering of scene
	WaterSurface,		// draw water surface using the reflection and refraction textures
	UI,					// 2D user interface
};

class CustomMeshRenderer;
namespace physics { class DebugDrawer; }

class CustomRenderContext : public RenderContext {
public:
	CustomRenderContext(Viewport& viewport) : RenderContext(viewport) {
	}

	static CustomRenderContext const& fromCtx(RenderContext const& r) {
		return dynamic_cast<CustomRenderContext const&>(r);
	}

	RenderPass renderPass = RenderPass::None;
	glm::vec4 clipPlane;
	bool enableClipPlane = false;
	bool cameraUnderwater = false;
	bool enableWaterRender = false;
	float time = 0;

	CustomMeshRenderer* meshRenderer = nullptr;
	physics::DebugDrawer* physDebugDraw = nullptr;
};

#endif // __CUST_RENDER_CONTEXT_H__
