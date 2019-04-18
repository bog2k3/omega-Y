#include "OffscreenRenderer.h"

#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/RenderContext.h>

#include <GL/glew.h>

struct OffscreenRenderer::PrivateData {
	std::unique_ptr<RenderContext> renderContext;
	Viewport viewport;

	FrameBuffer framebuffer;

	PrivateData(unsigned bufW, unsigned bufH, std::unique_ptr<RenderContext> &&renderContext)
		: renderContext(std::move(renderContext))
		, viewport(0, 0, bufW, bufH)
	{}
};

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
void OffscreenRenderer::begin() {

}

// end off-screen rendering and restore the previous pipeline state
void OffscreenRenderer::end() {

}

unsigned OffscreenRenderer::getFBTexture() const {
	return pData_->framebuffer.fbTextureId;
}
