#ifndef STATECONTROLLER_H
#define STATECONTROLLER_H

#include "GameState.h"

#include <boglfw/utils/Event.h>

class RenderContext;

class StateController
{
public:
	virtual ~StateController() {}

	virtual void update(float dt) {}
	virtual void draw(RenderContext const&) {}

	Event<void(GameState::StateNames)> onNewStateRequest;

protected:
	StateController(GameState &state) : state_(state) {}
	GameState& state_;
};

#endif // STATECONTROLLER_H
