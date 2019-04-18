#include "physics/CollisionChecker.h"
#include "physics/PhysBodyProxy.h"
#include "physics/DebugDrawer.h"

#include "entities/FreeCamera.h"
#include "entities/PlayerEntity.h"

#include "PlayerInputHandler.h"
#include "ImgDebugDraw.h"

#include "render/render.h"

#include "game/GameState.h"
#include "game/StateController.h"
#include "game/Session.h"

#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/renderOpenGL/Shape2D.h>
#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/renderOpenGL/Mesh.h>
#include <boglfw/renderOpenGL/MeshRenderer.h>
#include <boglfw/renderOpenGL/drawable.h>
#include <boglfw/input/GLFWInput.h>
#include <boglfw/input/InputEvent.h>
#include <boglfw/World.h>
#include <boglfw/OSD/SignalViewer.h>
#include <boglfw/GUI/GuiSystem.h>
#include <boglfw/Infrastructure.h>
#include <boglfw/utils/Event.h>

#include <boglfw/net/connection.h>
#include <boglfw/net/listener.h>

#include <boglfw/entities/Gizmo.h>
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
RenderConfig *pRenderCfg = nullptr;

PlayerInputHandler playerInputHandler;

Session *pSession = nullptr;
GameState *pCrtState = nullptr;

//Terrain* pTerrain = nullptr;
//TerrainConfig terrainConfig;
//SkyBox* pSkyBox = nullptr;

//PhysBodyProxy boxBodyMeta(nullptr);
//btBoxShape* boxShape = nullptr;
//btMotionState* boxMotionState = nullptr;
//Mesh* boxMesh = nullptr;

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
		if (!pSession)
			break;
		auto sCamCtrl = pSession->cameraCtrl().lock();
		if (sCamCtrl->getAttachedEntity().lock() == pSession->player().lock()) {
			sCamCtrl->attachToEntity(pSession->freeCam(), glm::vec3{0.f});
			playerInputHandler.setTargetObject(pSession->freeCam());
		} else {
			sCamCtrl->attachToEntity(pSession->player(), glm::vec3{0.f});
			playerInputHandler.setTargetObject(pSession->player());
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
		//pTerrain->generate(terrainConfig);
		//pTerrain->finishGenerate();

		// reset player
		//auto sPlayer = player.lock();
		//if (sPlayer)
		//	sPlayer->moveTo({0.f, terrainConfig.maxElevation + 10, 0.f});
	} break;
	case GLFW_KEY_Q:
		if (pRenderCfg)
			pRenderCfg->renderWireFrame = !pRenderCfg->renderWireFrame;
		//pTerrain->setWireframeMode(renderWireFrame);
	break;
	case GLFW_KEY_E:
		if (pRenderCfg)
			pRenderCfg->renderPhysicsDebug = !pRenderCfg->renderPhysicsDebug;
	break;
	case GLFW_KEY_X:
		/*World::getGlobal<ImgDebugDraw>()->setValues(pTerrain->getHeightField(), pTerrain->getGridSize().x, pTerrain->getGridSize().y,
			terrainConfig.minElevation, terrainConfig.maxElevation, ImgDebugDraw::FMT_GRAYSCALE, ImgDebugDraw::FILTER_LINEAR);
		World::getGlobal<ImgDebugDraw>()->enable();*/
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
	World::getGlobal<GuiSystem>()->handleInput(ev);
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

/*void physTestInit() {
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
}*/

/*void physTestDestroy() {
	if (boxBodyMeta.bodyPtr) {
		World::getGlobal<btDiscreteDynamicsWorld>()->removeRigidBody(boxBodyMeta.bodyPtr);
		delete boxBodyMeta.bodyPtr, boxBodyMeta.bodyPtr = nullptr;
		delete boxShape, boxShape = nullptr;
		delete boxMotionState, boxMotionState = nullptr;
	}
	delete boxMesh;
}*/

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

	if (pCrtState && pCrtState->name() == GameState::StateNames::GAMEPLAY) {
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

	GLText::get()->print("Omega-Y v0.2",
		{20, 20, ViewportCoord::absolute, ViewportCoord::bottom | ViewportCoord::left},
		20, glm::vec3(0.5f, 0.9, 1.0f));
}

void drawDebug(std::vector<drawable> &list, RenderContext const& ctx) {
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

	for (auto &d : list)
		d.draw(ctx);
	World::getGlobal<ImgDebugDraw>()->draw(ctx);
	drawDebugTexts();
}

void initPhysics(RenderData const& renderData) {
	// collision configuration contains default setup for memory , collision setup . Advanced users can create their own configuration .
	auto collisionConfig = new btDefaultCollisionConfiguration();
	// use the default collision dispatcher . For parallel processing you can use a diffent dispatcher ( see Extras / BulletMultiThreaded )
	auto dispatcher = new btCollisionDispatcher(collisionConfig);
	// btDbvtBroadphase is a good general purpose broadphase . You can also try out btAxis3Sweep
	auto broadphase = new btDbvtBroadphase();
	// the default constraint solver . For parallel processing you can use a different solver ( see Extras / BulletMultiThreaded )
	auto solver = new btSequentialImpulseConstraintSolver();
	btDiscreteDynamicsWorld *physWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
	physWorld->setDebugDrawer(renderData.renderCtx.physDebugDraw);
	physWorld->setGravity(btVector3(0, -9.8f, 0));

	World::setGlobal<btDiscreteDynamicsWorld>(physWorld);
}

void destroyPhysics() {
	delete World::getGlobal<btDiscreteDynamicsWorld>();
	World::setGlobal<btDiscreteDynamicsWorld>(nullptr);
}

void initWorld(RenderData &renderData) {
	WorldConfig wldCfg;
	wldCfg.drawBoundaries = false;
	World::setConfig(wldCfg);

	initPhysics(renderData);

	auto pImgDebugDraw = new ImgDebugDraw();
	World::setGlobal<ImgDebugDraw>(pImgDebugDraw);

	World::setGlobal<GuiSystem>(new GuiSystem(&renderData.viewport, {0.f, 0.f}, {renderData.windowW, renderData.windowH}));
}

/*void initTerrain(RenderData &renderData) {
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

	pTerrain->setWaterReflectionTex(renderData.waterRenderData.reflectionTex, pSkyBox->getCubeMapTexture());
	pTerrain->setWaterRefractionTex(renderData.waterRenderData.refractionTex);

	//BuildingGenerator::generate(BuildingsSettings{}, *pTerrain);

	renderData.renderCtx.meshRenderer->setWaterNormalTexture(pTerrain->getWaterNormalTexture());
}

void initSky() {
	pSkyBox = new SkyBox();
	pSkyBox->load("data/textures/sky/1");
}*/

bool iamhost = false;
net::listener netlistener;
std::vector<net::connection> netcons;

void newConnection(net::result result, net::connection connection) {
	if (result.code != net::result::ok) {
		//ERROR("Connection failed. Error code: " << result.code << "\nMessage: " << result.message);
		std::cerr << "Connection failed. Error code: " << result.code << "\nMessage: " << result.message << "\n";
		return;
	}
	//LOGLN("Connection established.");
	std::cout << "Connection established.\n";
	netcons.push_back(connection);
}

bool initNetwork(int argc, char** argv) {
	LOGPREFIX("NETWORK");
	auto printUsage = []() {
		LOGLN("Usage:\nOmegaY host portNumber\nOR\nOmegaY join host port");
	};
	if (argc < 2) {
		LOGLN("No arguments specified for networking.");
		return true;
	}
	if (!strcmp(argv[1], "host")) {
		if (argc < 3) {
			ERROR("Missing [port] argument for hosting.");
			printUsage();
			return false;
		}
		// start hosting
		LOGLN("Hosting on port " << argv[2] << " . . .");
		iamhost = true;
		netlistener = net::startListen(atoi(argv[2]), newConnection);
		return true;
	} else if (!strcmp(argv[1], "join")) {
		if (argc < 4) {
			ERROR("Missing remote address for connecting.");
			printUsage();
			return false;
		}
		// start joining
		LOGLN("Connecting to " << argv[2] << ":" << argv[3] << " . . .");
		iamhost = false;
		net::connect_async(argv[2], atoi(argv[3]), newConnection);
		return true;
	} else {
		LOGLN("Unknown argument " << argv[1]);
		return true;
	}
}

void stopNetwork() {
	if (iamhost)
		net::stopListen(netlistener);
	for (auto con : netcons)
		net::closeConnection(con);
}

//void initSession(Session::SessionType type) {
	// origin gizmo
	/*glm::mat4 gizmoTr = glm::translate(glm::mat4{1}, glm::vec3{0.f, 0.01f, 0.f});
	World::getInstance().takeOwnershipOf(std::make_shared<Gizmo>(gizmoTr, 1.f));

	// free camera
	auto sFreeCam = std::make_shared<FreeCamera>(glm::vec3{2.f, 1.f, 2.f}, glm::vec3{-1.f, -0.5f, -1.f});
	freeCam = sFreeCam;
	World::getInstance().takeOwnershipOf(sFreeCam);

	// camera controller (this one moves the render camera to the position of the target entity)
	auto sCamCtrl = std::make_shared<CameraController>(&pRenderData->viewport.camera());
	cameraCtrl = sCamCtrl;
	World::getInstance().takeOwnershipOf(sCamCtrl);
	sCamCtrl->attachToEntity(freeCam, {0.f, 0.f, 0.f});*/

	// player
	/*auto sPlayer = std::make_shared<PlayerEntity>(glm::vec3{0.f, terrainConfig.maxElevation + 10, 0.f}, 0.f);
	player = sPlayer;
	World::getInstance().takeOwnershipOf(sPlayer);

	sCamCtrl->attachToEntity(freeCam, {0.f, 0.f, 0.f});
	playerInputHandler.setTargetObject(freeCam);

	initSky();
	initTerrain(*pRenderData);

	physTestInit();*/
//}

void changeGameState(GameState::StateNames stateName) {
	if (pCrtState)
		delete pCrtState, pCrtState = nullptr;

	if (stateName == GameState::StateNames::EXIT_GAME) {
		signalQuit = true;
		return;
	}

	pCrtState = GameState::createState(stateName);

	pCrtState->controller().onNewStateRequest.add(changeGameState);
}

void updateStateCtrl(float dt) {
	pCrtState->controller().update(dt);
}

int main(int argc, char* argv[]) {
	perf::setCrtThreadName("main");

	int winW = 1280, winH = 900;
	RenderData renderData(winW, winH);
	pRenderCfg = &renderData.config;
	std::vector<drawable> drawDebugList;
	renderData.drawDebugData = std::bind(drawDebug, std::ref(drawDebugList), std::placeholders::_1);

	do {
		PERF_MARKER_FUNC;

		// initialize stuff:
		if (!initRender("Omega-Y", renderData)) {
			ERROR("Failed to initialize OpenGL / GLFW rendering system");
			return -1;
		}

		GLFWInput::initialize(gltGetWindow());
		GLFWInput::onInputEvent.add(onInputEventHandler);

		initWorld(renderData);

		//randSeed(1424118659);
		randSeed(time(NULL));
		LOGLN("RAND seed: " << rand_seed);

		SignalViewer sigViewer(
				{24, 4, ViewportCoord::percent, ViewportCoord::top|ViewportCoord::right},	// position
				{20, 10, ViewportCoord::percent}); 											// size
		drawDebugList.push_back(&sigViewer);

		UpdateList continuousUpdateList;
		continuousUpdateList.add(&sigViewer);
		continuousUpdateList.add(updateStateCtrl);

		UpdateList updateList;
		updateList.add(World::getGlobal<btDiscreteDynamicsWorld>());
		updateList.add(&CollisionChecker::update);
		updateList.add(&playerInputHandler);
		updateList.add(&World::getInstance());

		float realTime = 0;							// [s] real time that passed since starting
		float simulationTime = 0;					// [s] "simulation" or "in-game world" time that passed since starting - may be different when using slo-mo
		float frameRate = 0;

		sigViewer.addSignal("FPS", &frameRate,
				glm::vec3(1.f, 0.05f, 0.05f), 0.2f, 50, 0, 0, 0);

		changeGameState(GameState::StateNames::MAIN_MENU);

		// precache GPU resources by rendering the first frame before first update
		LOGLN("Precaching . . .");
		gltBegin();
		render(renderData);
		gltEnd();
		LOGLN("Done, we're now live.");

		if (!initNetwork(argc, argv)) {
			break;
		}

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
					renderData.renderCtx.time += simDT;
				}

				{
					PERF_MARKER("frame-draw");
					// wait until previous frame finishes rendering and show frame output:
					gltEnd();
					gltBegin();
					// start rendering the frame:
					render(renderData);
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
	} while (0);

	LOGLN("Exiting . . .");

	LOGLN("Closing all network connections . . .");
	stopNetwork();
	LOGLN("Deleting all entities . . .");
	World::getInstance().reset();
	LOGLN("Destroying physics . . .");
	destroyPhysics();
	if (auto ptr = World::getGlobal<ImgDebugDraw>()) {
		delete ptr;
		World::setGlobal<ImgDebugDraw>(nullptr);
	}
	unloadRender(renderData);
	Infrastructure::shutDown();

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
