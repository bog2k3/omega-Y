#ifndef __CUST_RENDER_CONTEXT_H__
#define __CUST_RENDER_CONTEXT_H__

#include <boglfw/renderOpenGL/RenderContext.h>

#include <glm/vec4.hpp>

#include <memory>

enum class RenderPass {
	None,
	WaterReflection,	// off-screen rendering for water reflection texture
	WaterRefraction,	// off-screen rendering for water refraction texture
	Standard,			// standard default rendering of scene
	WaterSurface,		// draw water surface using the reflection and refraction textures
	UI,					// 2D user interface
};

class CustomMeshRenderer;
class UPackCommon;
namespace physics { class DebugDrawer; }

class CustomRenderContext : public RenderContext {
public:
	static CustomRenderContext const& fromCtx(RenderContext const& r) {
		return dynamic_cast<CustomRenderContext const&>(r);
	}
	static CustomRenderContext& fromCtx(RenderContext& r) {
		return dynamic_cast<CustomRenderContext&>(r);
	}

	RenderPass renderPass = RenderPass::None;

	float subspace = 1.f;
	bool enableClipPlane = false;
	bool cameraUnderwater = false;
	bool enableWaterRender = false;
	float time = 0;

	CustomMeshRenderer* meshRenderer = nullptr;
	physics::DebugDrawer* physDebugDraw = nullptr;

	void updateCommonUniforms();
};

#endif // __CUST_RENDER_CONTEXT_H__
