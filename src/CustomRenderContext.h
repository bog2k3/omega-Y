#ifndef __CUST_RENDER_CONTEXT_H__
#define __CUST_RENDER_CONTEXT_H__

#include <boglfw/renderOpenGL/RenderContext.h>

#include <glm/vec4.hpp>

enum class RenderPass {
	None,
	WaterReflection,
	UnderWater,
	AboveWater,
	WaterSurface,
	UI,			// 2D user interface
};

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
};

#endif // __CUST_RENDER_CONTEXT_H__
