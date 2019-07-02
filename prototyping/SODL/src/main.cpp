#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/drawable.h>
#include <boglfw/renderOpenGL/GLText.h>
#include <boglfw/renderOpenGL/RenderContext.h>
#include <boglfw/renderOpenGL/RenderHelpers.h>
#include <boglfw/input/GLFWInput.h>
#include <boglfw/input/InputEvent.h>
#include <boglfw/World.h>
#include <boglfw/OSD/SignalViewer.h>
#include <boglfw/GUI/GuiSystem.h>
#include <boglfw/Infrastructure.h>
#include <boglfw/utils/log.h>
#include <boglfw/utils/UpdateList.h>
#include <boglfw/utils/rand.h>

#include <GLFW/glfw3.h>

bool signalQuit = false;

template<> void update(std::function<void(float)> *fn, float dt) {
	(*fn)(dt);
}

void handleSystemKeys(InputEvent& ev) {
	bool consumed = true;
	switch (ev.key) {
	case GLFW_KEY_ESCAPE:
		signalQuit = true;
		consumed = true;
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

void onInputEventHandler(InputEvent& ev) {
	// propagate input events in order of priority:
	if (!ev.isConsumed()) {
		if (ev.type == InputEvent::EV_KEY_DOWN)
			handleSystemKeys(ev);
	}
	if (!ev.isConsumed())
		handleGUIInputs(ev);
}

void drawDebugTexts(Viewport const& vp) {
	std::string texts[] {
		"ESC : Exit",
	};

	for (unsigned i=0; i<sizeof(texts)/sizeof(texts[0]); i++) {
		GLText::get()->print(texts[i],
			{20, 20 + 20*i},
			20, glm::vec3(0.4f, 0.6, 1.0f));
	}

	GLText::get()->print("GUI Test",
		FlexCoordPair(20, 20, FlexCoord::PIXELS, FlexCoordPair::LEFT, FlexCoordPair::BOTTOM).get(vp),
		20, glm::vec3(0.5f, 0.9, 1.0f));
}

void drawDebug(std::vector<drawable> &list, RenderContext const& ctx) {
	for (auto &d : list)
		d.draw(ctx);
	drawDebugTexts(ctx.viewport());
}

int main(int argc, char* argv[]) {
	int winW = 1280, winH = 900;
	
	// set up window
	if (!gltInitGLFW(winW, winH, "BOGLFW GUI Test", 0, false, true)) {
		ERROR("Failed to initialize GLFW Window and/or OpenGL context");
		return -1;
	}
	glFrontFace(GL_CW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	// load render helpers
	RenderHelpers::Config rcfg = RenderHelpers::defaultConfig();
	RenderHelpers::load(rcfg);
	
	Viewport viewport(0, 0, winW, winH);
	RenderContext renderCtx;
	renderCtx.pViewport = &viewport;

	do {
		LOGLN("Initializing subsystems...");
		
		const int margin = 20; // pixels
		World::setGlobal<GuiSystem>(new GuiSystem(&viewport, {margin, margin}, {winW - 2*margin, winH - 2*margin}));

		// initialize stuff:
		GLFWInput::initialize(gltGetWindow());
		GLFWInput::onInputEvent.add(onInputEventHandler);

		randSeed(time(NULL));
		
		std::vector<drawable> debugDrawList;

		SignalViewer sigViewer(
				{80, 4, FlexCoord::PERCENT},	// position
				{20, 10, FlexCoord::PERCENT});	// size
		debugDrawList.push_back(&sigViewer);

		UpdateList updateList;
		updateList.add(&sigViewer);
		updateList.add(&World::getInstance());

		float realTime = 0;							// [s] real time that passed since starting
		float simulationTime = 0;					// [s] "simulation" or "in-game world" time that passed since starting - may be different when using slo-mo
		float frameRate = 0;

		sigViewer.addSignal("FPS", &frameRate,
				glm::vec3(1.f, 0.05f, 0.05f), 0.2f, 50, 0, 0, 0);

		LOGLN("Done, we're now live.");

		// initial update:
		updateList.update(0);
		GLFWInput::resetInputQueue();	// purge any input events that occured during window creation

		float initialTime = glfwGetTime();
		float t = initialTime;
		float frameTime = 0;
		gltBegin();
		
		while (!signalQuit && GLFWInput::checkInput()) {
			float newTime = glfwGetTime();
			frameTime = 0.75 * frameTime + 0.25 * (newTime - t); // smooth out
			frameRate = 1.0 / frameTime;
			t = newTime;
			realTime = newTime - initialTime;

			// time step for simulation
			float maxFrameDT = 0.1f;	// cap the dt to avoid unwanted artifacts when framerate drops too much
			float simDT = min(maxFrameDT, frameTime);
			simulationTime += simDT;

			if (simDT > 0) {
				updateList.update(simDT);
			}

			// wait until previous frame finishes rendering and show frame output:
			gltEnd();
			gltBegin();
			// start rendering the frame:
			viewport.clear();
			viewport.render(&World::getInstance(), renderCtx);
			viewport.render(World::getGlobal<GuiSystem>(), renderCtx);
			viewport.render(debugDrawList, renderCtx);
			// now rendering is on-going, move on to the next update:
		}
	} while (0);

	LOGLN("Exiting . . .");
	gltShutDown();
	Infrastructure::shutDown();

	std::cout << "\n\n";

	return 0;
}
