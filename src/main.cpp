#include "entities/FreeCamera.h"
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

#include <rp3d/reactphysics3d.h>

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

std::weak_ptr<FreeCamera> freeCam;
PlayerInputHandler playerInputHandler;

Terrain* pTerrain = nullptr;
TerrainSettings terrainSettings;

template<> void update(std::function<void(float)> *fn, float dt) {
	(*fn)(dt);
}

template<> void update(rp3d::DynamicsWorld* wld, float dt) {
	// TODO fixed time step
	wld->update(dt);
}

bool toggleMouseCapture();

void handleSystemKeys(InputEvent& ev, bool &mouseCaptureDisabled) {
	if (ev.key == GLFW_KEY_ESCAPE) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			signalQuit = true;
			ev.consume();
		}
	} else if (ev.key == GLFW_KEY_F1) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			captureFrame = true;
			ev.consume();
		}
	} else if (ev.key == GLFW_KEY_F2) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			slowMo ^= true;
			ev.consume();
		}
	} else if (ev.key == GLFW_KEY_F3) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			updatePaused ^= true;
			ev.consume();
		}
	} else if (ev.key == GLFW_KEY_TAB) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			mouseCaptureDisabled = !toggleMouseCapture();
		}
	} else if (ev.key == GLFW_KEY_R) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			pTerrain->generate(terrainSettings);
		}
	} else if (ev.key == GLFW_KEY_Q) {
		if (ev.type == InputEvent::EV_KEY_DOWN) {
			renderWireFrame = !renderWireFrame;
			glPolygonMode(GL_FRONT_AND_BACK, renderWireFrame ? GL_LINE : GL_FILL);
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

void initSession(Camera* camera) {
	glm::mat4 gizmoTr = glm::translate(glm::mat4{1}, glm::vec3{0.f, 0.01f, 0.f});
	World::getInstance().takeOwnershipOf(std::make_shared<Gizmo>(gizmoTr, 1.f));
	//World::getInstance().takeOwnershipOf(std::make_shared<Box>(0.3f, 0.3f, 0.3f, glm::vec3{1.f, 0.5f, 0.f}));

	auto sFreeCam = std::make_shared<FreeCamera>(glm::vec3{4.3f, 1, 4}, glm::vec3{-4.3f, -0.5f, -4.f});
	freeCam = sFreeCam;
	World::getInstance().takeOwnershipOf(sFreeCam);
	
	auto sCamCtrl = std::make_shared<CameraController>(camera);
	World::getInstance().takeOwnershipOf(sCamCtrl);
	sCamCtrl->attachToEntity(freeCam, {0.f, 0.f, 0.f});
	
	playerInputHandler.setTargetObject(freeCam);
}

rp3d::RigidBody* groundBody = nullptr;
rp3d::BoxShape* groundShape = nullptr;
rp3d::RigidBody* boxBody = nullptr;
rp3d::BoxShape* boxShape = nullptr;
Mesh* boxMesh = nullptr;

void physTestInit(rp3d::DynamicsWorld &physWld) {
	return;
	// create ground body
	rp3d::Vector3 gPos(0.f, -0.1f, 0.f);
	rp3d::Quaternion gOrient = rp3d::Quaternion::identity();
	groundBody = physWld.createRigidBody({gPos, gOrient});
	groundBody->setType(rp3d::BodyType::STATIC);
	// create ground shape
	rp3d::Vector3 groundHalfExt(50, 0.1f, 50);
	groundShape = new rp3d::BoxShape(groundHalfExt);
	groundBody->addCollisionShape(groundShape, rp3d::Transform::identity(), 1.f);
	
	// create test body
	rp3d::Vector3 pos(0.f, 8.f, 0.f);
	rp3d::Quaternion orient({1.f, 0.5f, 1.f}, PI/3);
	boxBody = physWld.createRigidBody({pos, orient});
	// create test body shape
	rp3d::Vector3 boxHalfExt(0.5f, 0.5f, 0.5f);
	boxShape = new rp3d::BoxShape(boxHalfExt);
	boxBody->addCollisionShape(boxShape, rp3d::Transform::identity(), 10.f);
	// create test body representation
	boxMesh = new Mesh();
	boxMesh->createBox({0.f, 0.f, 0.f}, 1.f, 1.f, 1.f);
}

void physTestDebugDraw(Viewport* vp) {
	return;
	// draw ground grid
	float xext = 30;
	float zext = 30;
	float step = 0.5f;
	float y = 0.f;
	for (float x=-xext; x<xext; x+=step)
		Shape3D::get()->drawLine({x, y, -zext}, {x, y, +zext}, {1.f, 1.f, 1.f, 0.6f});
	for (float z=-zext; z<zext; z+=step)
		Shape3D::get()->drawLine({-xext, y, z}, {+xext, y, z}, {1.f, 1.f, 1.f, 0.6f});

	return;
		
	// draw the test body's representation:
	rp3d::Transform tr = boxBody->getTransform();
	glm::mat4 matTr;
	tr.getOpenGLMatrix(&matTr[0][0]);
	MeshRenderer::get()->renderMesh(*boxMesh, matTr);
		
	// draw info about simulated body
	rp3d::Transform bodyTr = boxBody->getTransform();
	auto pos = bodyTr.getPosition();
	std::stringstream ss;
	ss << "Body pos: " << pos.x << "; " << pos.y << "; " << pos.z;
	GLText::get()->print(ss.str(),
		{20, 40, ViewportCoord::absolute, ViewportCoord::top | ViewportCoord::left},
		0, 20, glm::vec3(1.f, 1.f, 1.f));
}

void physTestDestroy(rp3d::DynamicsWorld &physWld) {
	if (boxBody)
		physWld.destroyRigidBody(boxBody), boxBody = nullptr;
	if (groundBody)
		physWld.destroyRigidBody(groundBody), groundBody = nullptr;
}

int main(int argc, char* argv[]) {
	perf::setCrtThreadName("main");
	do {
		PERF_MARKER_FUNC;

		// initialize stuff:
		int winW = 1280, winH = 900;
		int multisamples = 4; // 0 to disable MSAA or >0 to enable it
		if (!gltInitGLFW(winW, winH, "Omega-Y", multisamples))
			return -1;

		GLFWInput::initialize(gltGetWindow());
		GLFWInput::onInputEvent.add(onInputEventHandler);

		Renderer renderer(winW, winH);
		auto vp = std::make_unique<Viewport>(0, 0, winW, winH);
		auto vp1 = vp.get();
		vp1->setBkColor({0.f, 0.f, 0.f});
		vp1->camera()->setFOV(PI/2.5f);
		vp1->camera()->setZPlanes(0.15f, 250.f);
		renderer.addViewport("main", std::move(vp));

		WorldConfig wldCfg;
		wldCfg.drawBoundaries = false;
		World::setConfig(wldCfg);
		World &world = World::getInstance();
		
		rp3d::Vector3 gravity(0.f, -9.81f, 0.f);
		rp3d::DynamicsWorld physWorld(gravity);
		physTestInit(physWorld);

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
		updateList.add(&physWorld);

		float realTime = 0;							// [s] real time that passed since starting
		float simulationTime = 0;					// [s] "simulation" or "in-game world" time that passed since starting - may be different when using slo-mo
		float frameRate = 0;

		sigViewer.addSignal("FPS", &frameRate,
				glm::vec3(1.f, 0.2f, 0.2f), 0.2f, 50, 0, 0, 0);

		auto infoTexts = [&](Viewport*) {
			GLText::get()->print("Omega-Y v0.1",
					{20, 20, ViewportCoord::absolute, ViewportCoord::bottom | ViewportCoord::left},
					0, 20, glm::vec3(0.5f, 0.9, 1.0f));
			GLText::get()->print("Press TAB to capture/release mouse",
					{20, 20, ViewportCoord::absolute, ViewportCoord::top | ViewportCoord::left},
					0, 20, glm::vec3(0.5f, 0.9, 1.0f));

			if (updatePaused) {
				GLText::get()->print("PAUSED",
						{50, 50, ViewportCoord::percent},
						0, 32, glm::vec3(1.f, 0.8f, 0.2f));
			}
			if (slowMo) {
				GLText::get()->print("~~ Slow Motion ON ~~",
						{10, 45},
						0, 18, glm::vec3(1.f, 0.5f, 0.1f));
			}
		};
		
		terrainSettings.vertexDensity = 1.f;	// vertices per meter
		terrainSettings.width = 200;
		terrainSettings.length = 200;
		terrainSettings.minElevation = -20;
		terrainSettings.maxElevation = 30.f;
		terrainSettings.relativeRandomJitter = 0.8f;
		terrainSettings.irregularEdges = true;
		terrainSettings.bigRoughness = 1.f;
		terrainSettings.smallRoughness = 1.f;
		Terrain terrain;
		pTerrain = &terrain;
		terrain.generate(terrainSettings);
		
		std::vector<drawable> drawList;
		drawList.push_back(&World::getInstance());
		drawList.push_back(&sigViewer);
		drawList.push_back(&infoTexts);
		drawList.push_back(&physTestDebugDraw);
		drawList.push_back(&terrain);
		
		vp1->setDrawList(drawList);
		
		initSession(vp1->camera());

		// initial update:
		updateList.update(0);
		GLFWInput::resetInputQueue();	// purge any input events that occured during window creation

		float initialTime = glfwGetTime();
		float t = initialTime;
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

		physTestDestroy(physWorld);
		renderer.unload();
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

