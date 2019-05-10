#ifndef STATECONTROLLER_H
#define STATECONTROLLER_H

#include "GameState.h"

#include <boglfw/utils/Event.h>

#include <map>

enum class StateSignals {
	ESCAPE,		// user pressed Esc key
	CLOSE,		// something happened and the current state needs to be closed and replaced.
};

class StateController
{
public:
	virtual ~StateController() {}

	virtual void update(float dt) {}
	// sends a signal to the controller
	void signal(StateSignals signal);

	Event<void(GameState::StateNames)> onNewStateRequest;

protected:
	StateController(GameState &state) : state_(state) {}
	GameState& state_;

	std::map<StateSignals, std::function<void()>> signalHandlers_;
};

#endif // STATECONTROLLER_H
