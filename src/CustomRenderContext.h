#ifndef __CUST_RENDER_CONTEXT_H__
#define __CUST_RENDER_CONTEXT_H__

#include <boglfw/renderOpenGL/RenderContext.h>

enum class RenderPass {
	None,
	UnderWater,
	AboveWater,
};

class CustomRenderContext : public RenderContext {
public:
	CustomRenderContext(Viewport& viewport) : RenderContext(viewport) {
	}

	RenderPass renderPass = RenderPass::None;
};

#endif // __CUST_RENDER_CONTEXT_H__
