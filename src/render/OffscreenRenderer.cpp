#include "OffscreenRenderer.h"

#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/RenderContext.h>

#include <GL/glew.h>

struct OffscreenRenderer::PrivateData {
	std::unique_ptr<RenderContext> renderContext;
	Viewport viewport;

	FrameBuffer framebuffer;

	unsigned previousFrameBuffer = 0;

	PrivateData(unsigned bufW, unsigned bufH, std::unique_ptr<RenderContext> &&renderContext)
		: renderContext(std::move(renderContext))
		, viewport(0, 0, bufW, bufH)
	{
		renderContext->pViewport = &viewport;
	}
};

const RenderContext& OffscreenRenderer::getRenderContext() const {
	return *pData_->renderContext.get();
}

OffscreenRenderer::OffscreenRenderer(FrameBufferDescriptor desc, std::unique_ptr<RenderContext> &&renderContext)
	: pData_(new PrivateData(desc.width, desc.height, std::move(renderContext)))
{
	gltCreateFrameBuffer(desc, pData_->framebuffer);
}

OffscreenRenderer::~OffscreenRenderer() {
	gltDestroyFrameBuffer(pData_->framebuffer);
	delete pData_;
}

// setup off-screen rendering
void OffscreenRenderer::render(std::vector<drawable> const& list) {
	// TODO...
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &pData_->previousFrameBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pData_->framebuffer.framebufferId);
	pData_->renderContext->viewport.
}

// clear the render target
void OffscreenRenderer::clear() {
	// TODO...
}

unsigned OffscreenRenderer::getFBTexture() const {
	return pData_->framebuffer.fbTextureId;
}
