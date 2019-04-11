#ifndef RENDER_H
#define RENDER_H

#include "CustomRenderContext.h"

#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/renderOpenGL/DefaultShaderPreprocessor.h>
#include <boglfw/renderOpenGL/RenderHelpers.h>
#include <boglfw/utils/assert.h>

#include <glm/fwd.hpp>

class Session;

struct PostProcessData {
	unsigned VAO = 0;
	unsigned VBO = 0;
	unsigned IBO = 0;
	unsigned shaderProgram = 0;
	int iTexSampler = 0;
	int iUnderwater = 0;
	int iTexSize = 0;
	int iTime = 0;

	glm::vec2 textureSize;

	std::vector<drawable> uiDrawList;
};

struct WaterRenderData {
	unsigned refractionFB = 0;
	unsigned refractionTex = 0;
	unsigned refractionDepth = 0;
	unsigned refractionFB_width = 0;
	unsigned refractionFB_height = 0;
	unsigned reflectionFB = 0;
	unsigned reflectionTex = 0;
	unsigned reflectionDepth = 0;
	unsigned reflectionFB_width = 0;
	unsigned reflectionFB_height = 0;

	glm::vec3 waterColor {0.06f, 0.16f, 0.2f};
};

struct RenderData {
	Viewport viewport;
	CustomRenderContext renderCtx;
	unsigned windowW = 0;
	unsigned windowH = 0;
	int defaultFrameBuffer = 0;

	std::function<void(RenderContext const& ctx)> drawDebugData;

	WaterRenderData waterRenderData;
	PostProcessData postProcessData;

	DefaultShaderPreprocessor shaderPreprocessor;

	RenderData(unsigned winW, unsigned winH)
		: viewport(0, 0, winW, winH)
		, renderCtx(viewport)
		, windowW(winW)
		, windowH(winH)
		{
		}

	~RenderData() {
		assertDbg(dependenciesUnloaded_ && "call unloadDependencies() before destroying this");
	}

private:
	friend bool initRender(const char*, RenderData&);
	friend void unloadRender(RenderData &renderData);

	void setupDependencies();
	void unloadDependencies();

	bool dependenciesUnloaded_ = true;
};

struct RenderConfig {
	bool renderWireFrame = false;
	bool renderPhysicsDebug = false;
};

bool initRender(const char* winTitle, RenderData &renderData);
void unloadRender(RenderData &renderData);
void render(RenderData &renderData, Session &session);


#endif // RENDER_H
