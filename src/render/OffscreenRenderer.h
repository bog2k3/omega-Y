#ifndef OFFSCREEN_RENDERER_H
#define OFFSCREEN_RENDERER_H

#include <boglfw/renderOpenGL/glToolkit.h>

#include <memory>

class RenderContext;

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

	// setup off-screen rendering
	void begin();
	// end off-screen rendering and restore the previous pipeline state
	void end();

	unsigned getFBTexture() const;

private:
	struct PrivateData;
	PrivateData* pData_ = nullptr;
};

#endif // OFFSCREEN_RENDERER_H
