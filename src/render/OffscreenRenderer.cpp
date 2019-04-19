#include "OffscreenRenderer.h"

#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/RenderContext.h>

#include <GL/glew.h>

#include <stdexcept>

struct OffscreenRenderer::PrivateData {
	std::unique_ptr<RenderContext> renderContext;
	Viewport viewport;

	FrameBuffer framebuffer;

	bool offscreenActive = false;

	PrivateData(unsigned bufW, unsigned bufH, std::unique_ptr<RenderContext> &&renderContext)
		: renderContext(std::move(renderContext))
		, viewport(0, 0, bufW, bufH)
	{
		this->renderContext->pViewport = &viewport;
	}
};

const RenderContext& OffscreenRenderer::getRenderContext() const {
	return *pData_->renderContext.get();
}

OffscreenRenderer::OffscreenRenderer(FrameBufferDescriptor desc, std::unique_ptr<RenderContext> &&renderContext)
	: pData_(new PrivateData(desc.width, desc.height, std::move(renderContext)))
{
	if (!pData_->framebuffer.create(desc))
		throw std::runtime_error("Unable to create off-screen framebuffer!");
}

OffscreenRenderer::~OffscreenRenderer() {
	delete pData_;
}

// setup off-screen rendering
void OffscreenRenderer::begin() {
	assertDbg(!pData_->offscreenActive && "OffscreenRenderer already active (calling begin() twice?)");
	pData_->framebuffer.bind();
	pData_->offscreenActive = true;
}

// restore the initial framebuffer config
void OffscreenRenderer::end() {
	assertDbg(pData_->offscreenActive && "OffscreenRenderer not active (calling end() twice?)");
	pData_->framebuffer.unbind();
	pData_->offscreenActive = false;
}

// render stuff
void OffscreenRenderer::render(drawable element) {
	assertDbg(pData_->offscreenActive && "OffscreenRenderer not active (forgot to call begin()?)");
	pData_->viewport.render(element, *pData_->renderContext);
}

// render stuff
void OffscreenRenderer::render(std::vector<drawable> const& list) {
	assertDbg(pData_->offscreenActive && "OffscreenRenderer not active (forgot to call begin()?)");
	pData_->viewport.render(list, *pData_->renderContext);
}

// clear the render target
void OffscreenRenderer::clear() {
	assertDbg(pData_->offscreenActive && "OffscreenRenderer not active (forgot to call begin()?)");
	pData_->viewport.clear();
}

unsigned OffscreenRenderer::getFBTexture() const {
	return pData_->framebuffer.fbTextureId();
}

Viewport& OffscreenRenderer::viewport() {
	return pData_->viewport;
}
