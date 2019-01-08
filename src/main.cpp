#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/Renderer.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/input/GLFWInput.h>
#include <boglfw/input/InputEvent.h>
#include <boglfw/World.h>
#include <boglfw/math/math3D.h>
#include <boglfw/OSD/SignalViewer.h>
#include <boglfw/GUI/GuiSystem.h>
#include <boglfw/Infrastructure.h>

#include <boglfw/utils/drawable.h>
#include <boglfw/utils/log.h>
#include <boglfw/utils/UpdateList.h>
#include <boglfw/utils/rand.h>

#include <boglfw/perf/marker.h>
#include <boglfw/perf/results.h>
#include <boglfw/perf/frameCapture.h>
#include <boglfw/perf/perfPrint.h>

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

template<> void update(std::function<void(float)> *fn, float dt) {
	(*fn)(dt);
}

void onInputEventHandler(InputEvent& ev) {
	if (ev.isConsumed())
		return;
	if (ev.key == GLFW_KEY_SPACE) {
		if (ev.type == InputEvent::EV_KEY_DOWN)
			updatePaused ^= true;
	} else if (ev.key == GLFW_KEY_S) {
		if (ev.type == InputEvent::EV_KEY_DOWN)
			slowMo ^= true;
	} else if (ev.key == GLFW_KEY_F1) {
		if (ev.type == InputEvent::EV_KEY_DOWN)
			captureFrame = true;
	}
}

int main(int argc, char* argv[]) {
	perf::setCrtThreadName("main");
	do {
		PERF_MARKER_FUNC;

		// initialize stuff:
		int winW = 1024, winH = 768;
		if (!gltInitGLFW(winW, winH, "OmegaY"))
			return -1;

		GLFWInput::initialize(gltGetWindow());
		GLFWInput::onInputEvent.add(onInputEventHandler);

		Renderer renderer(winW, winH);
		auto vp = std::make_unique<Viewport>(0, 0, winW, winH);
		auto vp1 = vp.get();
		vp1->setBkColor({0.f, 0.f, 0.f});
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

		float realTime = 0;							// [s]
		float simulationTime = 0;					// [s]
		float simDTAcc = 0; // [s] accumulated sim dt values since last status print
		float realDTAcc = 0; // [s] accumulated real dt values since last status print
		constexpr float simTimePrintInterval = 10.f; // [s]

		float frameTime = 0;

		sigViewer.addSignal("frameTime", &frameTime,
				glm::vec3(1.f, 0.2f, 0.2f), 0.1f, 50, 0.1, 0, 3);

		auto infoTexts = [&](Viewport*) {
			GLText::get()->print("OmegaY v0.1",
					{20, 20, ViewportCoord::absolute, ViewportCoord::bottom | ViewportCoord::left},
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

		// initial update:
		updateList.update(0);

		float t = glfwGetTime();
		while (GLFWInput::checkInput()) {
			if (captureFrame)
				perf::FrameCapture::start(perf::FrameCapture::AllThreads);
			/* frame context */
			{
				PERF_MARKER("frame");
				float newTime = glfwGetTime();
				float realDT = newTime - t;
				frameTime = realDT;
				realDTAcc += realDT;
				t = newTime;
				realTime += realDT;

				// fixed time step for simulation (unless slowMo is on)
				float simDT = updatePaused ? 0 : 0.02f;
				if (slowMo) {
					// use same fixed timestep in order to avoid breaking physics, but
					// only update once every n frames to slow down
					static float frameCounter = 0;
					constexpr float cycleLength = 10; // frames
					if (++frameCounter == cycleLength) {
						frameCounter = 0;
					} else
						simDT = 0;
				}

				simulationTime += simDT;
				simDTAcc += simDT;

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

