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

	void clear();
	void render(std::vector<drawable> const& list);

	unsigned getFBTexture() const;
	const RenderContext& getRenderContext() const;

private:
	struct PrivateData;
	PrivateData* pData_ = nullptr;
};

#endif // OFFSCREEN_RENDERER_H
