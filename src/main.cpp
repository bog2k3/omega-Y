#include "physics/DebugDrawer.h"
#include "entities/FreeCamera.h"
#include "entities/PlayerEntity.h"
#include "PlayerInputHandler.h"
#include "terrain/Terrain.h"

#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/Renderer.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/renderOpenGL/Mesh.h>
#include <boglfw/renderOpenGL/MeshRenderer.h>
#include <boglfw/renderOpenGL/shader.h>
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

#include <boglfw/utils/drawable.h>
#include <boglfw/utils/log.h>
#include <boglfw/utils/UpdateList.h>
#include <boglfw/utils/rand.h>

#include <boglfw/perf/marker.h>
#include <boglfw/perf/results.h>
#include <boglfw/perf/frameCapture.h>
#include <boglfw/perf/perfPrint.h>

#include <bullet3/btBulletDynamicsCommon.h>

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
TerrainSettings terrainSettings;

btRigidBody* boxBody = nullptr;
btBoxShape* boxShape = nullptr;
Mesh* boxMesh = nullptr;

template<> void update(std::function<void(float)> *fn, float dt) {
	(*fn)(dt);
}

template<> void update(btDiscreteDynamicsWorld* wld, float dt) {
	const float fixedTimeStep = 1.f / 100;	// 100Hz update rate for physics
	const int max_substeps = 10;
	wld->stepSimulation(dt, max_substeps, fixedTimeStep);
}

bool toggleMouseCapture();

void handleSystemKeys(InputEvent& ev, bool &mouseCaptureDisabled) {
	if (ev.key == GLFW_KEY_ESCAPE) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			ev.consume();
			signalQuit = true;
		}
	} else if (ev.key == GLFW_KEY_F1) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			ev.consume();
			mouseCaptureDisabled = !toggleMouseCapture();
		}
	} else if (ev.key == GLFW_KEY_F2) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			ev.consume();
			slowMo ^= true;
		}
	} else if (ev.key == GLFW_KEY_F3) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			ev.consume();
			updatePaused ^= true;
		}
	} else if (ev.key == GLFW_KEY_F10) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			ev.consume();
			captureFrame = true;
		}
	} else if (ev.key == GLFW_KEY_TAB) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			auto sCamCtrl = cameraCtrl.lock();
			if (sCamCtrl->getAttachedEntity().lock() == player.lock()) {
				sCamCtrl->attachToEntity(freeCam, glm::vec3{0.f});
				playerInputHandler.setTargetObject(freeCam);
			} else {
				sCamCtrl->attachToEntity(player, glm::vec3{0.f});
				playerInputHandler.setTargetObject(player);
			}
		}
	} else if (ev.key == GLFW_KEY_R) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			ev.consume();
			pTerrain->generate(terrainSettings);
			// reset the box:
			btQuaternion qOrient{0.5f, 0.13f, 1.2f};
			btVector3 vPos{2.f, terrainSettings.maxElevation + 10, 2.f};
			boxBody->setWorldTransform(btTransform{qOrient, vPos});
			// wake up the box:
			boxBody->activate(true);

			// reset player
			auto sPlayer = player.lock();
			if (sPlayer)
				sPlayer->moveTo({0.f, terrainSettings.maxElevation + 10, 0.f});
		}
	} else if (ev.key == GLFW_KEY_Q) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			ev.consume();
			renderWireFrame = !renderWireFrame;
			pTerrain->setWireframeMode(renderWireFrame);
		}
	} else if (ev.key == GLFW_KEY_E) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			ev.consume();
			renderPhysicsDebug = !renderPhysicsDebug;
		}
	}
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
	btVector3 vPos{2.f, terrainSettings.maxElevation + 10, 2.f};
	btRigidBody::btRigidBodyConstructionInfo cinfo {
		mass,
		nullptr,
		boxShape,
		boxInertia
	};
	cinfo.m_startWorldTransform = btTransform{qOrient, vPos};
	cinfo.m_linearDamping = 0.01f;
	cinfo.m_angularDamping = 0.01f;
	cinfo.m_friction = 0.4f;

	boxBody = new btRigidBody(cinfo);
	World::getGlobal<btDiscreteDynamicsWorld>()->addRigidBody(boxBody);

	boxMesh = new Mesh();
	boxMesh->createBox(glm::vec3{0.f}, 1.f, 1.f, 1.f);
}

void initSession(Camera* camera) {
	// origin gizmo
	glm::mat4 gizmoTr = glm::translate(glm::mat4{1}, glm::vec3{0.f, 0.01f, 0.f});
	World::getInstance().takeOwnershipOf(std::make_shared<Gizmo>(gizmoTr, 1.f));

	// free camera
	auto sFreeCam = std::make_shared<FreeCamera>(glm::vec3{4.3f, 1, 4}, glm::vec3{-4.3f, -0.5f, -4.f});
	freeCam = sFreeCam;
	World::getInstance().takeOwnershipOf(sFreeCam);

	// camera controller (this one moves the render camera to the position of the target entity)
	auto sCamCtrl = std::make_shared<CameraController>(camera);
	cameraCtrl = sCamCtrl;
	World::getInstance().takeOwnershipOf(sCamCtrl);

	// player
	auto sPlayer = std::make_shared<PlayerEntity>(glm::vec3{0.f, terrainSettings.maxElevation + 10, 0.f}, glm::vec3{0.f, 0.f, 1.f});
	player = sPlayer;
	World::getInstance().takeOwnershipOf(sPlayer);

	sCamCtrl->attachToEntity(freeCam, {0.f, 0.f, 0.f});
	playerInputHandler.setTargetObject(freeCam);

	physTestInit();
}

void physTestDebugDraw(Viewport* vp) {
	if (renderPhysicsDebug)
		World::getGlobal<btDiscreteDynamicsWorld>()->debugDrawWorld();
	// draw the test body's representation:
	btTransform tr = boxBody->getWorldTransform();
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
	if (boxBody) {
		World::getGlobal<btDiscreteDynamicsWorld>()->removeRigidBody(boxBody);
		delete boxBody, boxBody = nullptr;
		delete boxShape, boxShape = nullptr;
	}
	delete boxMesh;
}

void drawDebugTexts() {
	GLText::get()->print("TAB : toggle playerCam / freeCam",
			{20, 20, ViewportCoord::absolute, ViewportCoord::top | ViewportCoord::left},
			0, 20, glm::vec3(0.4f, 0.6, 1.0f));
	GLText::get()->print("R : regenerate terrain",
			{20, 40, ViewportCoord::absolute, ViewportCoord::top | ViewportCoord::left},
			0, 20, glm::vec3(0.4f, 0.6, 1.0f));
	GLText::get()->print("Q : toggle wireframe",
			{20, 60, ViewportCoord::absolute, ViewportCoord::top | ViewportCoord::left},
			0, 20, glm::vec3(0.4f, 0.6, 1.0f));
	GLText::get()->print("E : toggle debug drawing of physics",
			{20, 80, ViewportCoord::absolute, ViewportCoord::top | ViewportCoord::left},
			0, 20, glm::vec3(0.4f, 0.6, 1.0f));
	GLText::get()->print("F1 : capture / release mouse",
			{20, 100, ViewportCoord::absolute, ViewportCoord::top | ViewportCoord::left},
			0, 20, glm::vec3(0.4f, 0.6, 1.0f));
	GLText::get()->print("F2 : toggle slow motion",
			{20, 120, ViewportCoord::absolute, ViewportCoord::top | ViewportCoord::left},
			0, 20, glm::vec3(0.4f, 0.6, 1.0f));
	GLText::get()->print("F3 : pause",
			{20, 140, ViewportCoord::absolute, ViewportCoord::top | ViewportCoord::left},
			0, 20, glm::vec3(0.4f, 0.6, 1.0f));

	if (updatePaused) {
		GLText::get()->print("PAUSED",
				{50, 50, ViewportCoord::percent},
				0, 32, glm::vec3(1.f, 0.8f, 0.2f));
	}
	if (slowMo) {
		GLText::get()->print("~~ Slow Motion ON ~~",
				{50, 5, ViewportCoord::percent, ViewportCoord::top | ViewportCoord::left},
				0, 18, glm::vec3(1.f, 0.5f, 0.1f));
	}
}

struct {
	unsigned VAO;
	unsigned VBO;
	unsigned IBO;
	unsigned shaderProgram;
	unsigned iTexSampler;
} postProcessData;

bool initPostProcessData(unsigned winW, unsigned winH) {
	checkGLError();
	// load shader:
	postProcessData.shaderProgram = Shaders::createProgram("data/shaders/postprocess.vert", "data/shaders/postprocess.frag");
	if (!postProcessData.shaderProgram) {
		ERROR("Unabled to load post-processing shaders!");
		return false;
	}
	unsigned posAttrIndex = glGetAttribLocation(postProcessData.shaderProgram, "pos");
	unsigned uvAttrIndex = glGetAttribLocation(postProcessData.shaderProgram, "uv");
	postProcessData.iTexSampler = glGetUniformLocation(postProcessData.shaderProgram, "texSampler");

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
	glGenVertexArrays(1, &postProcessData.VAO);
	glBindVertexArray(postProcessData.VAO);
	glGenBuffers(1, &postProcessData.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, postProcessData.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuadPosUV), screenQuadPosUV, GL_STATIC_DRAW);
	glEnableVertexAttribArray(posAttrIndex);
	glVertexAttribPointer(posAttrIndex, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, 0);
	glEnableVertexAttribArray(uvAttrIndex);
	glVertexAttribPointer(uvAttrIndex, 2, GL_FLOAT, GL_FALSE, sizeof(float)*4, (void*)(sizeof(float)*2));
	glGenBuffers(1, &postProcessData.IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, postProcessData.IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(screenQuadIdx), screenQuadIdx, GL_STATIC_DRAW);
	glBindVertexArray(0);

	return !checkGLError("initPostProcessData");
}

void renderPostProcess() {
	glUseProgram(postProcessData.shaderProgram);
	glBindVertexArray(postProcessData.VAO);
	glUniform1i(postProcessData.iTexSampler, 0);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	glUseProgram(0);
	glBindVertexArray(0);
}

void deletePostProcessData() {
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

void initTerrain() {
	terrainSettings.vertexDensity = 1.f;	// vertices per meter
	terrainSettings.width = 200;
	terrainSettings.length = 200;
	terrainSettings.minElevation = -10;
	terrainSettings.maxElevation = 30.f;
	terrainSettings.seaLevel = 0.f;
	terrainSettings.relativeRandomJitter = 0.8f;
	terrainSettings.bigRoughness = 1.f;
	terrainSettings.smallRoughness = 1.f;
	pTerrain = new Terrain();
	pTerrain->generate(terrainSettings);
}

int main(int argc, char* argv[]) {
	perf::setCrtThreadName("main");
	do {
		PERF_MARKER_FUNC;

		// initialize stuff:
		int winW = 1280, winH = 900;
		if (!gltInitGLFW(winW, winH, "Omega-Y", 0, false))
			return -1;
		if (initPostProcessData(winW, winH)) {
			unsigned multisamples = 4; // >0 for MSSAA, 0 to disable
			gltSetPostProcessHook(PostProcessStep::POST_DOWNSAMPLING, renderPostProcess, multisamples);
		}

		GLFWInput::initialize(gltGetWindow());
		GLFWInput::onInputEvent.add(onInputEventHandler);

		Renderer renderer(winW, winH);
		auto vp = std::make_unique<Viewport>(0, 0, winW, winH);
		auto vp1 = vp.get();
		vp1->setBkColor({0.f, 0.f, 0.f});
		vp1->camera()->setFOV(PI/2.5f);
		vp1->camera()->setZPlanes(0.15f, 250.f);
		renderer.addViewport("main", std::move(vp));

		initWorld();
		World &world = World::getInstance();

		//randSeed(1424118659);
		randSeed(time(NULL));
		LOGLN("RAND seed: " << rand_seed);

		SignalViewer sigViewer(
				{24, 4, ViewportCoord::percent, ViewportCoord::top|ViewportCoord::right},	// position
				-0.1f, 																		// z
				{20, 10, ViewportCoord::percent}); 											// size

		UpdateList continuousUpdateList;

		UpdateList updateList;
		updateList.add(&World::getInstance());
		updateList.add(&sigViewer);
		updateList.add(&playerInputHandler);
		updateList.add(World::getGlobal<btDiscreteDynamicsWorld>());

		float realTime = 0;							// [s] real time that passed since starting
		float simulationTime = 0;					// [s] "simulation" or "in-game world" time that passed since starting - may be different when using slo-mo
		float frameRate = 0;

		sigViewer.addSignal("FPS", &frameRate,
				glm::vec3(1.f, 0.05f, 0.05f), 0.2f, 50, 0, 0, 0);

		auto infoTexts = [&](Viewport*) {
			GLText::get()->print("Omega-Y v0.1",
					{20, 20, ViewportCoord::absolute, ViewportCoord::bottom | ViewportCoord::left},
					0, 20, glm::vec3(0.5f, 0.9, 1.0f));
			drawDebugTexts();
		};

		initTerrain();

		std::vector<drawable> drawList;
		drawList.push_back(&World::getInstance());
		drawList.push_back(&sigViewer);
		drawList.push_back(&infoTexts);
		drawList.push_back(&physTestDebugDraw);
		drawList.push_back(pTerrain);

		vp1->setDrawList(drawList);

		initSession(vp1->camera());

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
				float simDT = updatePaused ? 0 : realDT;
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
					// build the render queue for the current frame and start the actual openGL render (which is independent of our world)
					renderer.render();
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

		world.reset();
		physTestDestroy();
		destroyPhysics();
		renderer.unload();
		deletePostProcessData();
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
