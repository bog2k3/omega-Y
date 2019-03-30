#include "CustomRenderContext.h"
#include "physics/DebugDrawer.h"
#include "physics/CollisionChecker.h"
#include "physics/PhysBodyProxy.h"
#include "entities/FreeCamera.h"
#include "entities/PlayerEntity.h"
#include "PlayerInputHandler.h"
#include "terrain/Terrain.h"
#include "sky/SkyBox.h"
#include "buildings/BuildingGenerator.h"
#include "ImgDebugDraw.h"
#include "render/frustum.h"

#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/renderOpenGL/Mesh.h>
#include <boglfw/renderOpenGL/MeshRenderer.h>
#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/renderOpenGL/drawable.h>
#include <boglfw/renderOpenGL/RenderHelpers.h>
#include <boglfw/input/GLFWInput.h>
#include <boglfw/input/InputEvent.h>
#include <boglfw/World.h>
#include <boglfw/math/math3D.h>
#include <boglfw/OSD/SignalViewer.h>
#include <boglfw/GUI/GuiSystem.h>
#include <boglfw/Infrastructure.h>

#include <boglfw/entities/Gizmo.h>
#include <boglfw/entities/Box.h>
#include <boglfw/entities/CameraController.h>

#include <boglfw/utils/log.h>
#include <boglfw/utils/UpdateList.h>
#include <boglfw/utils/rand.h>

#include <boglfw/perf/marker.h>
#include <boglfw/perf/results.h>
#include <boglfw/perf/frameCapture.h>
#include <boglfw/perf/perfPrint.h>

#include <bullet3/btBulletDynamicsCommon.h>
#include <bullet3/LinearMath/btDefaultMotionState.h>

#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <functional>
#include <stdexcept>
#include <cstdio>

#include <sys/stat.h>

#ifdef DEBUG_DMALLOC
#include <dmalloc.h>
#endif

bool updatePaused = false;
bool slowMo = false;
bool captureFrame = false;
bool signalQuit = false;
bool renderWireFrame = false;
bool renderPhysicsDebug = false;

physics::DebugDrawer* physDebugDraw = nullptr;

std::weak_ptr<FreeCamera> freeCam;
std::weak_ptr<PlayerEntity> player;
std::weak_ptr<CameraController> cameraCtrl;
PlayerInputHandler playerInputHandler;

Terrain* pTerrain = nullptr;
TerrainConfig terrainConfig;
SkyBox* pSkyBox = nullptr;

PhysBodyProxy boxBodyMeta(nullptr);
btBoxShape* boxShape = nullptr;
btMotionState* boxMotionState = nullptr;
Mesh* boxMesh = nullptr;

struct PostProcessData {
	unsigned VAO = 0;
	unsigned VBO = 0;
	unsigned IBO = 0;
	unsigned shaderProgram = 0;
	int iTexSampler = 0;
	int iUnderwater = 0;
	int iTexSize = 0;
	int iTime = 0;

	bool underwater = false;
	glm::vec2 textureSize;
	float time = 0;
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

	WaterRenderData waterRenderData;
	PostProcessData postProcessData;

	RenderData(unsigned winW, unsigned winH)
		: viewport(0, 0, winW, winH)
		, renderCtx(viewport)
		, windowW(winW)
		, windowH(winH)
		{}
};

template<> void update(std::function<void(float)> *fn, float dt) {
	(*fn)(dt);
}

template<> void update(btDiscreteDynamicsWorld* wld, float dt) {
	const float fixedTimeStep = 1.f / 60;	// 120Hz update rate for physics
	const int max_substeps = 10;
	wld->stepSimulation(dt, max_substeps, fixedTimeStep);
}

bool toggleMouseCapture();
void handleDebugKeys(InputEvent& ev);

void handleSystemKeys(InputEvent& ev, bool &mouseCaptureDisabled) {
	bool consumed = true;
	switch (ev.key) {
	case GLFW_KEY_ESCAPE:
		signalQuit = true;
	break;
	case GLFW_KEY_F1:
		mouseCaptureDisabled = !toggleMouseCapture();
	break;
	case GLFW_KEY_F10:
		captureFrame = true;
	break;
	case GLFW_KEY_TAB: {
		auto sCamCtrl = cameraCtrl.lock();
		if (sCamCtrl->getAttachedEntity().lock() == player.lock()) {
			sCamCtrl->attachToEntity(freeCam, glm::vec3{0.f});
			playerInputHandler.setTargetObject(freeCam);
		} else {
			sCamCtrl->attachToEntity(player, glm::vec3{0.f});
			playerInputHandler.setTargetObject(player);
		}
	} break;
	default:
		consumed = false;
	}
	if (consumed)
		ev.consume();
#ifdef DEBUG
	else
		handleDebugKeys(ev);
#endif
}

void handleDebugKeys(InputEvent& ev) {
	bool consumed = true;
	switch (ev.key) {
	case GLFW_KEY_F2:
		slowMo ^= true;
	break;
	case GLFW_KEY_F3:
		updatePaused ^= true;
	break;
	case GLFW_KEY_R: {
		pTerrain->generate(terrainConfig);
		pTerrain->finishGenerate();
		// reset the box:
		btQuaternion qOrient{0.5f, 0.13f, 1.2f};
		btVector3 vPos{2.f, terrainConfig.maxElevation + 10, 2.f};
		boxBodyMeta.bodyPtr->setWorldTransform(btTransform{qOrient, vPos});
		// wake up the box:
		boxBodyMeta.bodyPtr->activate(true);

		// reset player
		auto sPlayer = player.lock();
		if (sPlayer)
			sPlayer->moveTo({0.f, terrainConfig.maxElevation + 10, 0.f});
	} break;
	case GLFW_KEY_Q:
		renderWireFrame = !renderWireFrame;
		pTerrain->setWireframeMode(renderWireFrame);
	break;
	case GLFW_KEY_E:
		renderPhysicsDebug = !renderPhysicsDebug;
	break;
	case GLFW_KEY_X:
		World::getGlobal<ImgDebugDraw>()->setValues(pTerrain->getHeightField(), pTerrain->getGridSize().x, pTerrain->getGridSize().y,
			terrainConfig.minElevation, terrainConfig.maxElevation, ImgDebugDraw::FMT_GRAYSCALE, ImgDebugDraw::FILTER_LINEAR);
		World::getGlobal<ImgDebugDraw>()->enable();
	break;
	case GLFW_KEY_Z:
		World::getGlobal<ImgDebugDraw>()->disable();
	break;
	case GLFW_KEY_P:
		Shaders::reloadAllShaders();
	break;
	default:
		consumed = false;
	}
	if (consumed)
		ev.consume();
}

void handleGUIInputs(InputEvent& ev) {
	// call guiSystem.handleInput(ev)
}

void handlePlayerInputs(InputEvent& ev) {
	playerInputHandler.handleInputEvent(ev);
}

void onInputEventHandler(InputEvent& ev) {
	// suppress first mouse move event because the values are broken
	static bool firstMoveSuppressed = false;
	if (!firstMoveSuppressed && ev.type == InputEvent::EV_MOUSE_MOVED) {
		ev.consume();
		firstMoveSuppressed = true;
		return;
	}
	// propagate input events in order of priority:
	if (!ev.isConsumed()) {
		bool mouseUncaptured = false;
		if (ev.type == InputEvent::EV_KEY_DOWN)
			handleSystemKeys(ev, mouseUncaptured);
		if (mouseUncaptured)
			firstMoveSuppressed = false; // enable suppressing first move event again after uncapturing mouse to avoid jumps
	}
	if (!ev.isConsumed())
		handleGUIInputs(ev);
	if (!ev.isConsumed())
		handlePlayerInputs(ev);
}

bool toggleMouseCapture() {
	static bool isCaptured = false;
	if (isCaptured)
		glfwSetInputMode(gltGetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	else
		glfwSetInputMode(gltGetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	isCaptured = !isCaptured;
	return isCaptured;
}

void physTestInit() {
	// create test body
	boxShape = new btBoxShape({0.5f, 0.5f, 0.5f});
	float mass = 100.f;
	btVector3 boxInertia;
	boxShape->calculateLocalInertia(mass, boxInertia);
	btQuaternion qOrient{0.5f, 0.13f, 1.2f};
	btVector3 vPos{2.f, terrainConfig.maxElevation + 10, 2.f};
	boxMotionState = new btDefaultMotionState(btTransform{qOrient, vPos});
	btRigidBody::btRigidBodyConstructionInfo cinfo {
		mass,
		boxMotionState,
		boxShape,
		boxInertia
	};
	cinfo.m_linearDamping = 0.01f;
	cinfo.m_angularDamping = 0.01f;
	cinfo.m_friction = 0.4f;

	boxBodyMeta.bodyPtr = new btRigidBody(cinfo);
	boxBodyMeta.bodyPtr->setUserPointer(&boxBodyMeta);
	World::getGlobal<btDiscreteDynamicsWorld>()->addRigidBody(boxBodyMeta.bodyPtr);

	boxMesh = new Mesh();
	boxMesh->createBox(glm::vec3{0.f}, 1.f, 1.f, 1.f);
}

void initSession(Camera& camera) {
	// origin gizmo
	glm::mat4 gizmoTr = glm::translate(glm::mat4{1}, glm::vec3{0.f, 0.01f, 0.f});
	World::getInstance().takeOwnershipOf(std::make_shared<Gizmo>(gizmoTr, 1.f));

	// free camera
	auto sFreeCam = std::make_shared<FreeCamera>(glm::vec3{4.3f, 20, 4}, glm::vec3{-4.3f, -1.5f, -4.f});
	freeCam = sFreeCam;
	World::getInstance().takeOwnershipOf(sFreeCam);

	// camera controller (this one moves the render camera to the position of the target entity)
	auto sCamCtrl = std::make_shared<CameraController>(&camera);
	cameraCtrl = sCamCtrl;
	World::getInstance().takeOwnershipOf(sCamCtrl);

	// player
	auto sPlayer = std::make_shared<PlayerEntity>(glm::vec3{0.f, terrainConfig.maxElevation + 10, 0.f}, 0.f);
	player = sPlayer;
	World::getInstance().takeOwnershipOf(sPlayer);

	sCamCtrl->attachToEntity(freeCam, {0.f, 0.f, 0.f});
	playerInputHandler.setTargetObject(freeCam);

	physTestInit();
}

void physTestDebugDraw(RenderContext const& ctx) {
	if (renderPhysicsDebug)
		World::getGlobal<btDiscreteDynamicsWorld>()->debugDrawWorld();
	// draw the test body's representation:
	btTransform tr;
	boxMotionState->getWorldTransform(tr);
	glm::mat4 matTr;
	tr.getOpenGLMatrix(&matTr[0][0]);
	MeshRenderer::get()->renderMesh(*boxMesh, matTr);

	// draw info about simulated body
	/*rp3d::Transform bodyTr = boxBody->getTransform();
	auto pos = bodyTr.getPosition();
	std::stringstream ss;
	ss << "Body pos: " << pos.x << "; " << pos.y << "; " << pos.z;
	GLText::get()->print(ss.str(),
		{20, 100, ViewportCoord::absolute, ViewportCoord::top | ViewportCoord::left},
		0, 20, glm::vec3(1.f, 1.f, 1.f));*/
}

void physTestDestroy() {
	if (boxBodyMeta.bodyPtr) {
		World::getGlobal<btDiscreteDynamicsWorld>()->removeRigidBody(boxBodyMeta.bodyPtr);
		delete boxBodyMeta.bodyPtr, boxBodyMeta.bodyPtr = nullptr;
		delete boxShape, boxShape = nullptr;
		delete boxMotionState, boxMotionState = nullptr;
	}
	delete boxMesh;
}

void drawDebugTexts() {
	std::string texts[] {
		"TAB : toggle playerCam / freeCam",
		"F1 : capture / release mouse",
#ifdef DEBUG
		"R : regenerate terrain",
		"Q : toggle wireframe",
		"E : toggle debug drawing of physics",
		"F2 : toggle slow motion",
		"F3 : pause",
		"X : debug terrain heightmap",
		"Z : disable debug image",
		"P : reload all shaders"
#endif
	};
	for (unsigned i=0; i<sizeof(texts)/sizeof(texts[0]); i++) {
		GLText::get()->print(texts[i],
			{20, 20 + 20*i, ViewportCoord::absolute, ViewportCoord::top | ViewportCoord::left},
			20, glm::vec3(0.4f, 0.6, 1.0f));
	}

	if (updatePaused) {
		GLText::get()->print("PAUSED",
				{50, 50, ViewportCoord::percent},
				32, glm::vec3(1.f, 0.8f, 0.2f));
	}
	if (slowMo) {
		GLText::get()->print("~~ Slow Motion ON ~~",
				{50, 5, ViewportCoord::percent, ViewportCoord::top | ViewportCoord::left},
				18, glm::vec3(1.f, 0.5f, 0.1f));
	}
}

void drawDebug(RenderContext const& ctx) {
	/*Frustum f(ctx.viewport.camera().matProjView());
	Trapezoid t = projectFrustum(f, {0.f, 1.f, 0.f, 0.f}, 200);
	auto center = glm::vec2(renderData.windowW/2, renderData.windowH/2);
	Shape2D::get()->drawRectangleCentered(center, {400.f, 400.f}, {0.f, 1.f, 0.f});
	Shape2D::get()->drawLine(center - glm::vec2{10, 0}, center + glm::vec2{10, 0}, {0.f, 1.f, 0.f});
	Shape2D::get()->drawLine(center - glm::vec2{0, 10}, center + glm::vec2{0, 10}, {0.f, 1.f, 0.f});
	for (int i=0; i<4; i++)
		t.v[i].z *= -1;
	for (int i=0; i<4; i++) {
		int inext = (i+1) % 4;
		auto p1 = center + vec3xz(t.v[i]) * 2;
		auto p2 = center + vec3xz(t.v[inext]) * 2;
		Shape2D::get()->drawLine(p1, p2, {0.5f, 1.f, 0.f});
	}*/
}

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

void renderPostProcess(PostProcessData &postProcessData) {
	checkGLError("renderPostProcess 0");
	// do the post-processing render
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindVertexArray(postProcessData.VAO);
	checkGLError("renderPostProcess 0a");
	glUseProgram(postProcessData.shaderProgram);
	checkGLError("renderPostProcess 0b");
	glUniform1i(postProcessData.iTexSampler, 0);
	glUniform1i(postProcessData.iUnderwater, postProcessData.underwater ? 1 : 0);
	glUniform2fv(postProcessData.iTexSize, 1, &postProcessData.textureSize.x);
	glUniform1f(postProcessData.iTime, postProcessData.time);
	checkGLError("renderPostProcess 1");
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	glUseProgram(0);
	glBindVertexArray(0);

	checkGLError("renderPostProcess 3");
}

void deletePostProcessData(PostProcessData &postProcessData) {
	glDeleteBuffers(1, &postProcessData.VBO);
	glDeleteBuffers(1, &postProcessData.IBO);
	glDeleteVertexArrays(1, &postProcessData.VAO);
}

void initPhysics() {
	// collision configuration contains default setup for memory , collision setup . Advanced users can create their own configuration .
	auto collisionConfig = new btDefaultCollisionConfiguration();
	// use the default collision dispatcher . For parallel processing you can use a diffent dispatcher ( see Extras / BulletMultiThreaded )
	auto dispatcher = new btCollisionDispatcher(collisionConfig);
	// btDbvtBroadphase is a good general purpose broadphase . You can also try out btAxis3Sweep
	auto broadphase = new btDbvtBroadphase();
	// the default constraint solver . For parallel processing you can use a different solver ( see Extras / BulletMultiThreaded )
	auto solver = new btSequentialImpulseConstraintSolver();
	btDiscreteDynamicsWorld *physWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
	physWorld->setDebugDrawer(physDebugDraw = new physics::DebugDrawer());
	physWorld->setGravity(btVector3(0, -9.8f, 0));

	World::setGlobal<btDiscreteDynamicsWorld>(physWorld);
}

void destroyPhysics() {
	delete World::getGlobal<btDiscreteDynamicsWorld>();
	World::setGlobal<btDiscreteDynamicsWorld>(nullptr);
}

void initWorld() {
	WorldConfig wldCfg;
	wldCfg.drawBoundaries = false;
	World::setConfig(wldCfg);

	initPhysics();
}

void initTerrain(WaterRenderData &waterRenderData) {
	terrainConfig.vertexDensity = 1.f;	// vertices per meter
	terrainConfig.width = 200;
	terrainConfig.length = 200;
	terrainConfig.minElevation = -10;
	terrainConfig.maxElevation = 35.f;
	terrainConfig.relativeRandomJitter = 0.8f;

	terrainConfig.bigRoughness = 0.9f;
	terrainConfig.smallRoughness = 0.5f;
	pTerrain = new Terrain();
	pTerrain->generate(terrainConfig);
	pTerrain->finishGenerate();

	pTerrain->setWaterReflectionTex(waterRenderData.reflectionTex, pSkyBox->getCubeMapTexture());
	pTerrain->setWaterRefractionTex(waterRenderData.refractionTex);

	//BuildingGenerator::generate(BuildingsSettings{}, *pTerrain);
}

void initSky() {
	pSkyBox = new SkyBox();
	pSkyBox->load("data/textures/sky/1");
}

bool initRender(int winW, int winH, const char* winTitle, RenderData* &out_renderData) {
	// set up window
	if (!gltInitGLFW(winW, winH, winTitle, 0, false))
		return false;
	out_renderData = new RenderData(winW, winH);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// set up post processing hook
	if (initPostProcessData(winW, winH, out_renderData->postProcessData)) {
		unsigned multisamples = 4; // >0 for MSSAA, 0 to disable
		gltSetPostProcessHook(PostProcessStep::POST_DOWNSAMPLING, std::bind(renderPostProcess, std::ref(out_renderData->postProcessData)), multisamples);
	}

	// set up water refraction framebuffer
	out_renderData->waterRenderData.refractionFB_width = winW / 2;
	out_renderData->waterRenderData.refractionFB_height = winH / 2;
	if (!gltCreateFrameBuffer(out_renderData->waterRenderData.refractionFB_width, out_renderData->waterRenderData.refractionFB_height, GL_RGBA16, 0,
								out_renderData->waterRenderData.refractionFB, out_renderData->waterRenderData.refractionTex,
								&out_renderData->waterRenderData.refractionDepth))
		return false;
	glBindTexture(GL_TEXTURE_2D, out_renderData->waterRenderData.refractionTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// set up water reflection framebuffer
	out_renderData->waterRenderData.reflectionFB_width = winW / 2;
	out_renderData->waterRenderData.reflectionFB_height = winH / 2;
	if (!gltCreateFrameBuffer(out_renderData->waterRenderData.reflectionFB_width, out_renderData->waterRenderData.reflectionFB_height, GL_RGBA8, 0,
								out_renderData->waterRenderData.reflectionFB, out_renderData->waterRenderData.reflectionTex,
								&out_renderData->waterRenderData.reflectionDepth))
		return false;
	glBindTexture(GL_TEXTURE_2D, out_renderData->waterRenderData.reflectionTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// load render helpers
	RenderHelpers::Config rcfg = RenderHelpers::defaultConfig();
	RenderHelpers::load(rcfg);

	// set up viewport and camera
	out_renderData->viewport.setBkColor({0.f, 0.f, 0.f});
	out_renderData->viewport.camera().setFOV(PI/2.5f);
	out_renderData->viewport.camera().setZPlanes(0.15f, 500.f);

	// done
	return true;
}

void unloadRender(RenderData* &renderData) {
	deletePostProcessData(renderData->postProcessData);
	glDeleteTextures(1, &renderData->waterRenderData.refractionTex);
	glDeleteRenderbuffers(1, &renderData->waterRenderData.refractionDepth);
	glDeleteFramebuffers(1, &renderData->waterRenderData.refractionFB);
	delete renderData, renderData = nullptr;
	RenderHelpers::unload();
	glfwDestroyWindow(gltGetWindow());
}

void setupRenderPass(RenderData &renderData) {
	switch (renderData.renderCtx.renderPass) {
	case RenderPass::WaterReflection:
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderData.waterRenderData.reflectionFB);
		renderData.viewport.setArea(0, 0, renderData.waterRenderData.reflectionFB_width, renderData.waterRenderData.reflectionFB_height);
		renderData.viewport.setBkColor(renderData.waterRenderData.waterColor);
		renderData.viewport.clear();
		renderData.renderCtx.clipPlane = {0.f, renderData.renderCtx.cameraUnderwater ? -1.f : +1.f, 0.f, 0.f};
		renderData.renderCtx.enableClipPlane = true;
		renderData.viewport.camera().mirror(renderData.renderCtx.clipPlane);
	break;
	case RenderPass::WaterRefraction: {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderData.waterRenderData.refractionFB);
		renderData.viewport.setArea(0, 0, renderData.waterRenderData.refractionFB_width, renderData.waterRenderData.refractionFB_height);
		renderData.viewport.setBkColor(glm::vec4(0.07f, 0.16f, 0.2f, 1.f));
		renderData.viewport.clear();
		renderData.viewport.setBkColor(glm::vec3(0.f));
		renderData.renderCtx.clipPlane = {0.f, renderData.renderCtx.cameraUnderwater ? +1.f : -1.f, 0.f, 0.f};
		renderData.renderCtx.enableClipPlane = true;
		renderData.viewport.camera().mirror(renderData.renderCtx.clipPlane);
	} break;
	case RenderPass::Standard: {
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderData.defaultFrameBuffer);
		renderData.renderCtx.clipPlane = {0.f, renderData.renderCtx.cameraUnderwater  ? -1.f : +1.f, 0.f, 0.f};
		renderData.renderCtx.enableClipPlane = true;
		renderData.viewport.setArea(0, 0, renderData.windowW, renderData.windowH);
		if (renderData.renderCtx.cameraUnderwater) {
			renderData.viewport.setBkColor(renderData.waterRenderData.waterColor);
			renderData.viewport.clear();
		}
	} break;
	case RenderPass::WaterSurface:
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderData.defaultFrameBuffer);
		renderData.viewport.setArea(0, 0, renderData.windowW, renderData.windowH);
		renderData.renderCtx.enableClipPlane = false;
	break;
	case RenderPass::UI:
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderData.defaultFrameBuffer);
		renderData.viewport.setArea(0, 0, renderData.windowW, renderData.windowH);
	break;
	}
}

void render(RenderData &renderData, std::vector<drawable> &drawlist3D, std::vector<drawable> &drawlist2D) {
	renderData.viewport.clear();

	std::vector<drawable> underDraw = drawlist3D;
	std::vector<Entity*> underEntities;
	// append all drawable entities from world:
	// TODO - use a BSP or something to only get entities under water level
	World::getInstance().getEntities(underEntities, nullptr, 0, Entity::FunctionalityFlags::DRAWABLE);
	for (auto e : underEntities)
		underDraw.push_back(e);

	std::vector<drawable> aboveDraw { pSkyBox };
	aboveDraw.insert(aboveDraw.end(), drawlist3D.begin(), drawlist3D.end());
	std::vector<Entity*> aboveEntities;
	// append all drawable entities from world:
	// TODO - use a BSP or something to only get entities above water level
	World::getInstance().getEntities(aboveEntities, nullptr, 0, Entity::FunctionalityFlags::DRAWABLE);
	for (auto e : aboveEntities)
		aboveDraw.push_back(e);

	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &renderData.defaultFrameBuffer);
	renderData.renderCtx.cameraUnderwater = renderData.viewport.camera().position().y < 0;

	checkGLError("render() setup");

	// 1st pass - reflection
	renderData.renderCtx.renderPass = RenderPass::WaterReflection;
	setupRenderPass(renderData);
	renderData.viewport.render(renderData.renderCtx.cameraUnderwater ? underDraw : aboveDraw);

	checkGLError("render() pass #1");

	// 2nd pass - refraction
	renderData.renderCtx.renderPass = RenderPass::WaterRefraction;
	setupRenderPass(renderData);
	renderData.viewport.render(renderData.renderCtx.cameraUnderwater ? aboveDraw : underDraw);

	checkGLError("render() pass #2");

	// 3rd pass - standard rendering of scene
	renderData.renderCtx.renderPass = RenderPass::Standard;
	setupRenderPass(renderData);
	renderData.viewport.render(renderData.renderCtx.cameraUnderwater ? underDraw : aboveDraw);

	checkGLError("render() pass #3");

	// 4th pass - water surface
	renderData.renderCtx.renderPass = RenderPass::WaterSurface;
	setupRenderPass(renderData);
	renderData.viewport.render({pTerrain});

	checkGLError("render() pass #4");

	// last - 2D UI
	renderData.renderCtx.renderPass = RenderPass::UI;
	setupRenderPass(renderData);
	renderData.viewport.render(drawlist2D);

	checkGLError("render() final");

	renderData.renderCtx.renderPass = RenderPass::None;

	renderData.postProcessData.underwater = renderData.renderCtx.cameraUnderwater;
}

int main(int argc, char* argv[]) {
	perf::setCrtThreadName("main");
	do {
		PERF_MARKER_FUNC;

		// initialize stuff:
		int winW = 1280, winH = 900;
		RenderData* pRenderData = nullptr;
		if (!initRender(winW, winH, "Omega-Y", pRenderData)) {
			ERROR("Failed to initialize OpenGL / GLFW rendering system");
			return -1;
		}

		GLFWInput::initialize(gltGetWindow());
		GLFWInput::onInputEvent.add(onInputEventHandler);

		initWorld();

		//randSeed(1424118659);
		randSeed(time(NULL));
		LOGLN("RAND seed: " << rand_seed);

		auto pImgDebugDraw = new ImgDebugDraw();
		World::setGlobal<ImgDebugDraw>(pImgDebugDraw);

		initSky();
		initTerrain(pRenderData->waterRenderData);

		SignalViewer sigViewer(
				{24, 4, ViewportCoord::percent, ViewportCoord::top|ViewportCoord::right},	// position
				{20, 10, ViewportCoord::percent}); 											// size

		UpdateList continuousUpdateList;
		continuousUpdateList.add(&sigViewer);

		UpdateList updateList;
		updateList.add(World::getGlobal<btDiscreteDynamicsWorld>());
		updateList.add(&CollisionChecker::update);
		updateList.add(&playerInputHandler);
		updateList.add(&World::getInstance());
		updateList.add(pSkyBox);
		updateList.add(pTerrain);

		auto postProcessUpdate = [pRenderData](float dt) {
			pRenderData->postProcessData.time += dt;
		};
		updateList.add(&postProcessUpdate);

		float realTime = 0;							// [s] real time that passed since starting
		float simulationTime = 0;					// [s] "simulation" or "in-game world" time that passed since starting - may be different when using slo-mo
		float frameRate = 0;

		sigViewer.addSignal("FPS", &frameRate,
				glm::vec3(1.f, 0.05f, 0.05f), 0.2f, 50, 0, 0, 0);

		auto infoTexts = [&](RenderContext const&) {
			GLText::get()->print("Omega-Y v0.1",
					{20, 20, ViewportCoord::absolute, ViewportCoord::bottom | ViewportCoord::left},
					20, glm::vec3(0.5f, 0.9, 1.0f));
			drawDebugTexts();
		};

		std::vector<drawable> drawList3D;
		drawList3D.push_back(&physTestDebugDraw);
		drawList3D.push_back(pTerrain);

		std::vector<drawable> drawList2D;
		drawList2D.push_back(&sigViewer);
		drawList2D.push_back(&infoTexts);
		drawList2D.push_back(drawDebug);
		drawList2D.push_back(pImgDebugDraw);

		initSession(pRenderData->viewport.camera());

		// precache GPU resources by rendering the first frame before first update
		LOGLN("Precaching . . .");
		gltBegin();
		render(*pRenderData, drawList3D, drawList2D);
		gltEnd();
		LOGLN("Done, we're now live.");

		// initial update:
		updateList.update(0);
		GLFWInput::resetInputQueue();	// purge any input events that occured during window creation

		float initialTime = glfwGetTime();
		float t = initialTime;
		gltBegin();
		while (!signalQuit && GLFWInput::checkInput()) {
			if (captureFrame)
				perf::FrameCapture::start(perf::FrameCapture::AllThreads);
			/* frame context */
			{
				PERF_MARKER("frame");
				float newTime = glfwGetTime();
				float realDT = newTime - t;
				frameRate = 1.0 / realDT;
				t = newTime;
				realTime = newTime - initialTime;

				// time step for simulation
				float maxFrameDT = 0.1f;	// cap the dt to avoid unwanted artifacts when framerate drops too much
				float simDT = min(maxFrameDT, updatePaused ? 0 : realDT);
				if (slowMo) {
					simDT *= 0.1f;	// 10x slow-down factor
				}

				simulationTime += simDT;

				continuousUpdateList.update(realDT);
				if (simDT > 0) {
					PERF_MARKER("frame-update");
					updateList.update(simDT);
				}

				{
					PERF_MARKER("frame-draw");
					// wait until previous frame finishes rendering and show frame output:
					gltEnd();
					gltBegin();
					// start rendering the frame:
					render(*pRenderData, drawList3D, drawList2D);
					// now rendering is on-going, move on to the next update:
				}
			} /* frame context */

			if (captureFrame) {
				captureFrame = false;
				perf::FrameCapture::stop();
				printFrameCaptureData(perf::FrameCapture::getResults());
				perf::FrameCapture::cleanup();
			}
		}

		World::getInstance().reset();
		physTestDestroy();
		destroyPhysics();
		if (auto ptr = World::getGlobal<ImgDebugDraw>()) {
			delete ptr;
			World::setGlobal<ImgDebugDraw>(nullptr);
		}
		unloadRender(pRenderData);
		Infrastructure::shutDown();
	} while (0);

	if (false) {
		// print profiling stats
		for (unsigned i=0; i<perf::Results::getNumberOfThreads(); i++) {
			std::cout << "\n=============Call Tree for thread [" << perf::Results::getThreadName(i) << "]==========================================\n";
			printCallTree(perf::Results::getCallTrees(i), 0);
			std::cout << "\n------------ TOP HITS -------------\n";
			printTopHits(perf::Results::getFlatList(i));
			std::cout << "\n--------------- END -------------------------------\n";
		}
	}

	std::cout << "\n\n";

	return 0;
}
