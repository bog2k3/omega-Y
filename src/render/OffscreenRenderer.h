#ifndef OFFSCREEN_RENDERER_H
#define OFFSCREEN_RENDERER_H

#include <boglfw/renderOpenGL/Framebuffer.h>
#include <boglfw/renderOpenGL/drawable.h>

#include <memory>
#include <vector>

class RenderContext;
class Viewport;

/*
	Use this convenience class to render to off-screen buffers/textures.
	Do all the rendering as usual, between begin() and end() methods.
	Use getFBTexture() to get the texture ID of the off-screen frame buffer.
	You need to provide a unique RenderContext (or your custom extension of that) as a parameter on the constructor.
*/
class OffscreenRenderer {
public:
	OffscreenRenderer(FrameBufferDescriptor descriptor, std::unique_ptr<RenderContext> &&renderContext);
	~OffscreenRenderer();

	// set everything up for off-screen rendering
	void begin();
	// clear the render target (call this only between begin() and end())
	void clear();
	// render some stuff into the offscreen buffer (call this only between begin() and end())
	void render(std::vector<drawable> const& list);
	void render(drawable element);
	// restore the previous framebuffer configuration
	void end();

	Viewport& viewport();
	unsigned getFBTexture() const;
	const RenderContext& getRenderContext() const;

private:
	struct PrivateData;
	PrivateData* pData_ = nullptr;
};

#endif // OFFSCREEN_RENDERER_H
