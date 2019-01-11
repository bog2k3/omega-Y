#include "entities/FreeCamera.h"
#include "PlayerInputHandler.h"

#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/Renderer.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/Camera.h>
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

std::weak_ptr<FreeCamera> freeCam;
PlayerInputHandler playerInputHandler;

template<> void update(std::function<void(float)> *fn, float dt) {
	(*fn)(dt);
}

void toggleMouseCapture();

void handleSystemKeys(InputEvent& ev) {
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
			toggleMouseCapture();
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
	// propagate input events in order of priority:
	if (!ev.isConsumed())
		handleSystemKeys(ev);
	if (!ev.isConsumed())
		handleGUIInputs(ev);
	if (!ev.isConsumed())
		handlePlayerInputs(ev);
}

void toggleMouseCapture() {
	static bool isCaptured = false;
	if (isCaptured)
		glfwSetInputMode(gltGetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	else
		glfwSetInputMode(gltGetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	isCaptured = !isCaptured;
}

void initSession(Camera* camera) {
	World::getInstance().takeOwnershipOf(std::make_shared<Gizmo>(glm::mat4{1.f}, 1.f));
	World::getInstance().takeOwnershipOf(std::make_shared<Box>(0.3f, 0.3f, 0.3f, glm::vec3{1.f, 0.5f, 0.f}));

	auto sFreeCam = std::make_shared<FreeCamera>(glm::vec3{2.3f, 1, 2}, glm::vec3{-2.3f, -1, -2});
	freeCam = sFreeCam;
	World::getInstance().takeOwnershipOf(sFreeCam);
	
	auto sCamCtrl = std::make_shared<CameraController>(camera);
	World::getInstance().takeOwnershipOf(sCamCtrl);
	sCamCtrl->attachToEntity(freeCam, {0.f, 0.f, 0.f});
	
	playerInputHandler.setTargetObject(freeCam);
}

btDiscreteDynamicsWorld* physWorld = nullptr;

void initPhysTest() {
	auto collisionConfig = new btDefaultCollisionConfiguration();
	auto dispatcher = new btCollisionDispatcher(collisionConfig);
	auto broadphase = new btDbvtBroadphase();
	auto solver = new btSequentialImpulseConstraintSolver();
	physWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfig);
	//physWorld->setGravity(btVector3(0, -10, 0));
}

int main(int argc, char* argv[]) {
	perf::setCrtThreadName("main");
	do {
		PERF_MARKER_FUNC;

		// initialize stuff:
		int winW = 1024, winH = 768;
		if (!gltInitGLFW(winW, winH, "Omega-Y"))
			return -1;

		GLFWInput::initialize(gltGetWindow());
		GLFWInput::onInputEvent.add(onInputEventHandler);

		Renderer renderer(winW, winH);
		auto vp = std::make_unique<Viewport>(0, 0, winW, winH);
		auto vp1 = vp.get();
		vp1->setBkColor({0.f, 0.f, 0.f});
		vp1->camera()->setFOV(PI/2.5f);
		renderer.addViewport("main", std::move(vp));

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
		
		std::vector<drawable> drawList;
		drawList.push_back(&World::getInstance());
		drawList.push_back(&sigViewer);
		drawList.push_back(&infoTexts);
		
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

		renderer.unload();
		Infrastructure::shutDown();
	} while (0);

	for (unsigned i=0; i<perf::Results::getNumberOfThreads(); i++) {
		std::cout << "\n=============Call Tree for thread [" << perf::Results::getThreadName(i) << "]==========================================\n";
		printCallTree(perf::Results::getCallTrees(i), 0);
		std::cout << "\n------------ TOP HITS -------------\n";
		printTopHits(perf::Results::getFlatList(i));
		std::cout << "\n--------------- END -------------------------------\n";
	}

	std::cout << "\n\n";

	return 0;
}

