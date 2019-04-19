#include "render.h"
#include "CustomMeshRenderer.h"
#include "../physics/DebugDrawer.h"
#include "../terrain/Water.h"

#include <boglfw/utils/log.h>
#include <boglfw/math/math3D.h>
#include <boglfw/entities/Entity.h>
#include <boglfw/World.h>
#include <boglfw/GUI/GuiSystem.h>

#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <GL/gl.h>

void renderPostProcess(RenderData &renderData);

void RenderData::setupDependencies() {
	Shaders::useShaderPreprocessor(&shaderPreprocessor);
	renderCtx.meshRenderer = new CustomMeshRenderer();
	renderCtx.physDebugDraw = new physics::DebugDrawer();

	dependenciesUnloaded_ = false;
}

void RenderData::unloadDependencies() {
	Shaders::useShaderPreprocessor(nullptr);
	delete renderCtx.meshRenderer, renderCtx.meshRenderer = nullptr;
	delete renderCtx.physDebugDraw, renderCtx.physDebugDraw = nullptr;

	dependenciesUnloaded_ = true;
}

/*void physTestDebugDraw(RenderContext const& ctx) {
	if (renderPhysicsDebug)
		World::getGlobal<btDiscreteDynamicsWorld>()->debugDrawWorld();
	// draw the test body's representation:
	btTransform tr;
	boxMotionState->getWorldTransform(tr);
	glm::mat4 matTr;
	tr.getOpenGLMatrix(&matTr[0][0]);
	//MeshRenderer::get()->renderMesh(*boxMesh, matTr);
	CustomRenderContext::fromCtx(ctx).meshRenderer->renderMesh(*boxMesh, matTr, ctx);

	// draw info about simulated body
	/*rp3d::Transform bodyTr = boxBody->getTransform();
	auto pos = bodyTr.getPosition();
	std::stringstream ss;
	ss << "Body pos: " << pos.x << "; " << pos.y << "; " << pos.z;
	GLText::get()->print(ss.str(),
		{20, 100, ViewportCoord::absolute, ViewportCoord::top | ViewportCoord::left},
		0, 20, glm::vec3(1.f, 1.f, 1.f));* /
}*/

bool initPostProcessData(unsigned winW, unsigned winH, PostProcessData &postProcessData) {
	checkGLError();
	glGenVertexArrays(1, &postProcessData.VAO);
	glGenBuffers(1, &postProcessData.VBO);
	glGenBuffers(1, &postProcessData.IBO);
	PostProcessData *pPostProcessData = &postProcessData; // need this to avoid a compiler bug where capturing a reference by reference will result in UB
	// load shader:
	Shaders::createProgram("data/shaders/postprocess.vert", "data/shaders/postprocess.frag", [pPostProcessData](unsigned id) {
		pPostProcessData->shaderProgram = id;
		if (!pPostProcessData->shaderProgram) {
			ERROR("Unabled to load post-processing shaders!");
			return;
		}
		unsigned posAttrIndex = glGetAttribLocation(pPostProcessData->shaderProgram, "pos");
		unsigned uvAttrIndex = glGetAttribLocation(pPostProcessData->shaderProgram, "uv");
		pPostProcessData->iTexSampler = glGetUniformLocation(pPostProcessData->shaderProgram, "texSampler");
		pPostProcessData->iUnderwater = glGetUniformLocation(pPostProcessData->shaderProgram, "underwater");
		pPostProcessData->iTexSize = glGetUniformLocation(pPostProcessData->shaderProgram, "texSize_inv");
		pPostProcessData->iTime = glGetUniformLocation(pPostProcessData->shaderProgram, "time");

		glBindVertexArray(pPostProcessData->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, pPostProcessData->VBO);
		glEnableVertexAttribArray(posAttrIndex);
		glVertexAttribPointer(posAttrIndex, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, 0);
		glEnableVertexAttribArray(uvAttrIndex);
		glVertexAttribPointer(uvAttrIndex, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)(sizeof(float)*2));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pPostProcessData->IBO);
		glBindVertexArray(0);
		checkGLError("Postprocess shader");
	});
	if (!postProcessData.shaderProgram)
		return false;

	postProcessData.textureSize = glm::vec2(1.f / winW, 1.f / winH);

	// create screen quad:
	float screenQuadPosUV[] {
		-1.f, -1.f, 0.f, 0.f, 	// bottom-left
		-1.f, +1.f, 0.f, 1.f, 	// top-left
		+1.f, +1.f, 1.f, 1.f, 	// top-right
		+1.f, -1.f, 1.f, 0.f, 	// bottom-right
	};
	uint16_t screenQuadIdx[] {
		0, 1, 2, 0, 2, 3
	};

	glBindBuffer(GL_ARRAY_BUFFER, postProcessData.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuadPosUV), screenQuadPosUV, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, postProcessData.IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(screenQuadIdx), screenQuadIdx, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return !checkGLError("initPostProcessData");
}

void deletePostProcessData(PostProcessData &postProcessData) {
	LOGPREFIX("RENDER");
	glDeleteBuffers(1, &postProcessData.VBO);
	glDeleteBuffers(1, &postProcessData.IBO);
	glDeleteVertexArrays(1, &postProcessData.VAO);
}

bool initRender(const char* winTitle, RenderData &renderData) {
	LOGPREFIX("RENDER");
	// set up window
	if (!gltInitGLFW(renderData.windowW, renderData.windowH, winTitle, 0, false))
		return false;
	renderData.setupDependencies();

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	// configure backface culling
	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// set up post processing hook
	if (initPostProcessData(renderData.windowW, renderData.windowH, renderData.postProcessData)) {
		unsigned multisamples = 4; // >0 for MSSAA, 0 to disable
		gltSetPostProcessHook(PostProcessStep::POST_DOWNSAMPLING, std::bind(renderPostProcess, std::ref(renderData)), multisamples);
	}

	// set up water refraction framebuffer
	renderData.waterRenderData.refractionFBDesc.width = renderData.windowW / 2;
	renderData.waterRenderData.refractionFBDesc.height = renderData.windowH / 2;
	renderData.waterRenderData.refractionFBDesc.format = GL_RGBA16;
	renderData.waterRenderData.refractionFBDesc.multisamples = 0;
	renderData.waterRenderData.refractionFBDesc.requireDepthBuffer = true;
	if (!renderData.waterRenderData.refractionFramebuffer.create(renderData.waterRenderData.refractionFBDesc))
		return false;
	glBindTexture(GL_TEXTURE_2D, renderData.waterRenderData.refractionFramebuffer.fbTextureId());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// set up water reflection framebuffer
	renderData.waterRenderData.reflectionFBDesc.width = renderData.windowW / 2;
	renderData.waterRenderData.reflectionFBDesc.height = renderData.windowH / 2;
	renderData.waterRenderData.reflectionFBDesc.format = GL_RGBA8;
	renderData.waterRenderData.reflectionFBDesc.multisamples = 0;
	renderData.waterRenderData.reflectionFBDesc.requireDepthBuffer = true;;
	if (!renderData.waterRenderData.reflectionFramebuffer.create(renderData.waterRenderData.reflectionFBDesc))
		return false;
	glBindTexture(GL_TEXTURE_2D, renderData.waterRenderData.reflectionFramebuffer.fbTextureId());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// load render helpers
	RenderHelpers::Config rcfg = RenderHelpers::defaultConfig();
	RenderHelpers::load(rcfg);

	// set up viewport and camera
	renderData.viewport.setBkColor({0.f, 0.f, 0.f});
	renderData.viewport.camera().setFOV(PI/2.5f);
	renderData.viewport.camera().setZPlanes(0.15f, 1000.f);

	// done
	return true;
}

void unloadRender(RenderData &renderData) {
	LOGPREFIX("RENDER");
	deletePostProcessData(renderData.postProcessData);
	renderData.waterRenderData.reflectionFramebuffer.destroy();
	renderData.waterRenderData.refractionFramebuffer.destroy();
	renderData.unloadDependencies();
	RenderHelpers::unload();
	gltShutDown();
}

void setupRenderPass(RenderData &renderData, RenderPass pass) {
	renderData.renderCtx.renderPass = pass;

	LOGPREFIX("RENDER");
	float waterDepthFactor = pow(1.f / (max(0.f, -renderData.viewport.camera().position().y) + 1), 0.5f);
	switch (renderData.renderCtx.renderPass) {
	case RenderPass::WaterReflection:
		renderData.waterRenderData.reflectionFramebuffer.bind();
		renderData.viewport.setArea(0, 0, renderData.waterRenderData.reflectionFBDesc.width, renderData.waterRenderData.reflectionFBDesc.height);
		renderData.viewport.setBkColor(renderData.waterRenderData.waterColor * waterDepthFactor);
		renderData.viewport.clear();
		renderData.renderCtx.clipPlane = {0.f, renderData.renderCtx.cameraUnderwater ? -1.f : +1.f, 0.f, 0.f};
		renderData.renderCtx.enableClipPlane = true;
		renderData.viewport.camera().mirror(renderData.renderCtx.clipPlane);
	break;
	case RenderPass::WaterRefraction: {
		renderData.waterRenderData.refractionFramebuffer.bind();
		renderData.viewport.setArea(0, 0, renderData.waterRenderData.refractionFBDesc.width, renderData.waterRenderData.refractionFBDesc.height);
		renderData.viewport.setBkColor(glm::vec4(0.07f, 0.16f, 0.2f, 1.f));
		renderData.viewport.clear();
		renderData.renderCtx.clipPlane = {0.f, renderData.renderCtx.cameraUnderwater ? +1.f : -1.f, 0.f, 0.f};
		renderData.renderCtx.enableClipPlane = true;
	} break;
	case RenderPass::Standard: {
		renderData.renderCtx.clipPlane = {0.f, renderData.renderCtx.cameraUnderwater  ? -1.f : +1.f, 0.f, 0.f};
		renderData.renderCtx.enableClipPlane = true;
		if (renderData.renderCtx.cameraUnderwater) {
			renderData.viewport.setBkColor(renderData.waterRenderData.waterColor * waterDepthFactor);
			renderData.viewport.clear();
		}
	} break;
	case RenderPass::WaterSurface:
	break;
	case RenderPass::UI:
		glDisable(GL_DEPTH_TEST);
	break;
	case RenderPass::None:
	break;
	}

	if (renderData.renderCtx.enableClipPlane)
		glEnable(GL_CLIP_DISTANCE0);
	else
		glDisable(GL_CLIP_DISTANCE0);
}

void resetRenderPass(RenderData &renderData, RenderPass pass) {
	renderData.renderCtx.renderPass = RenderPass::None;
	renderData.renderCtx.enableClipPlane = false;
	renderData.renderCtx.cameraUnderwater = false;

	switch(pass) {
		case RenderPass::Standard:
			renderData.viewport.setBkColor(glm::vec3(0.f));
		break;
		case RenderPass::UI:
			glEnable(GL_DEPTH_TEST);
		break;
		case RenderPass::WaterReflection:
			renderData.waterRenderData.reflectionFramebuffer.unbind();
			renderData.viewport.setArea(0, 0, renderData.windowW, renderData.windowH);
			renderData.viewport.camera().mirror(renderData.renderCtx.clipPlane);
			renderData.viewport.setBkColor(glm::vec3(0.f));
		break;
		case RenderPass::WaterRefraction:
			renderData.waterRenderData.refractionFramebuffer.unbind();
			renderData.viewport.setBkColor(glm::vec3(0.f));
		break;
		case RenderPass::WaterSurface:
		break;
	}
}

void renderPostProcess(RenderData &renderData) {
	LOGPREFIX("RENDER");
	PostProcessData &postProcessData = renderData.postProcessData;

	checkGLError("renderPostProcess 0");
	// do the post-processing render
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindVertexArray(postProcessData.VAO);
	checkGLError("renderPostProcess 1");
	glUseProgram(postProcessData.shaderProgram);
	checkGLError("renderPostProcess 2");
	glUniform1i(postProcessData.iTexSampler, 0);
	glUniform1i(postProcessData.iUnderwater, renderData.renderCtx.cameraUnderwater ? 1 : 0);
	glUniform2fv(postProcessData.iTexSize, 1, &postProcessData.textureSize.x);
	glUniform1f(postProcessData.iTime, renderData.renderCtx.time);
	checkGLError("renderPostProcess 3");
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	glUseProgram(0);
	glBindVertexArray(0);

	checkGLError("renderPostProcess after");

	// now we render the UI and 2D stuff
	setupRenderPass(renderData, RenderPass::UI);
	renderData.viewport.render(World::getGlobal<GuiSystem>(), renderData.renderCtx);
	renderData.viewport.render(&renderData.drawDebugData, renderData.renderCtx);
	resetRenderPass(renderData, RenderPass::UI);

	checkGLError("render UI");
}

void render(RenderData &renderData) {
	LOGPREFIX("RENDER");

	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &renderData.defaultFrameBuffer);
	//auto &drawlist3D = session.drawList3D();
	//auto &drawList2D = session.drawList2D();

	if (renderData.renderCtx.enableWaterRender) {
		std::vector<drawable> underDraw;// = drawlist3D;
		std::vector<Entity*> underEntities;
		// append all drawable entities from world:
		// TODO - use a BSP or something to only get entities under water level
		World::getInstance().getEntities(underEntities, nullptr, 0, Entity::FunctionalityFlags::DRAWABLE);
		for (auto e : underEntities)
			underDraw.push_back(e);

		std::vector<drawable> aboveDraw {};
		//aboveDraw.insert(aboveDraw.end(), drawlist3D.begin(), drawlist3D.end());
		std::vector<Entity*> aboveEntities;
		// append all drawable entities from world:
		// TODO - use a BSP or something to only get entities above water level
		World::getInstance().getEntities(aboveEntities, nullptr, 0, Entity::FunctionalityFlags::DRAWABLE);
		for (auto e : aboveEntities)
			aboveDraw.push_back(e);

		renderData.renderCtx.cameraUnderwater = renderData.viewport.camera().position().y < 0;

		checkGLError("render() setup");

		// 1st pass - reflection
		setupRenderPass(renderData, RenderPass::WaterReflection);
		renderData.viewport.render(renderData.renderCtx.cameraUnderwater ? underDraw : aboveDraw, renderData.renderCtx);
		resetRenderPass(renderData, RenderPass::WaterReflection);

		checkGLError("render() pass #1");

		// 2nd pass - refraction
		setupRenderPass(renderData, RenderPass::WaterRefraction);
		renderData.viewport.render(renderData.renderCtx.cameraUnderwater ? aboveDraw : underDraw, renderData.renderCtx);
		resetRenderPass(renderData, RenderPass::WaterRefraction);

		checkGLError("render() pass #2");

		// 3rd pass - standard rendering of scene
		setupRenderPass(renderData, RenderPass::Standard);
		renderData.viewport.render(renderData.renderCtx.cameraUnderwater ? underDraw : aboveDraw, renderData.renderCtx);
		resetRenderPass(renderData, RenderPass::Standard);
	} else {
		// no water, just render everything in one pass:
		setupRenderPass(renderData, RenderPass::Standard);
		renderData.viewport.render(&World::getInstance(), renderData.renderCtx);
		//renderData.viewport.render(drawlist3D, renderData.renderCtx);
		resetRenderPass(renderData, RenderPass::Standard);
	}

	checkGLError("render() pass #3");

	// 4th pass - water surface
	if (renderData.renderCtx.enableWaterRender) {
		setupRenderPass(renderData, RenderPass::WaterSurface);
		//session.pWater_->draw(renderData.renderCtx);
		resetRenderPass(renderData, RenderPass::WaterSurface);

		checkGLError("render() pass #4");
	}

	// 2D UI will be rendered after post processing

	checkGLError("render() final");
}
