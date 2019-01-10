#include "PlayerInputHandler.h"
#include "entities/IUserControllable.h"

#include <boglfw/input/InputEvent.h>

#include <GLFW/glfw3.h>

PlayerInputHandler::PlayerInputHandler() {
	setDefaultBindings();
}

PlayerInputHandler::~PlayerInputHandler() {
}

void PlayerInputHandler::setDefaultBindings() {
	bindings[MOVE_FORWARD]		= { GLFW_KEY_W, false, bindingDescriptor::DeviceType::Keyboard };
	bindings[MOVE_BACKWARD]		= { GLFW_KEY_S, false, bindingDescriptor::DeviceType::Keyboard };
	bindings[MOVE_LEFT] 		= { GLFW_KEY_A, false, bindingDescriptor::DeviceType::Keyboard };
	bindings[MOVE_RIGHT] 		= { GLFW_KEY_D, false, bindingDescriptor::DeviceType::Keyboard };
	bindings[RUN]				= { GLFW_KEY_LEFT_SHIFT, false, bindingDescriptor::DeviceType::Keyboard };
	bindings[MOVE_UP] 			= { InputEvent::MB_RIGHT, false, bindingDescriptor::DeviceType::Mouse };
	bindings[MOVE_DOWN] 		= { InputEvent::MB_LEFT, false, bindingDescriptor::DeviceType::Mouse };
	bindings[ROTATE_YAW] 		= { 0, true, bindingDescriptor::DeviceType::Mouse };	// mouse x axis
	bindings[ROTATE_PITCH] 		= { 1, true, bindingDescriptor::DeviceType::Mouse };	// mouse y axis
}

void PlayerInputHandler::handleInputEvent(InputEvent& ev) {
	// for now we don't have a player, just a free camera
	for (int a=0; a < ALL_ACTIONS; a++) {
		switch (bindings[a].device) {
			case bindingDescriptor::DeviceType::Keyboard: {
				// treat keyboard buttons
				if (ev.key == bindings[a].code) {
					if (ev.type == InputEvent::EV_KEY_DOWN)
						inputStates_[a] = 1.f;					// TODO change to += 1.f if allowing multiple bindings for same action
					else if (ev.type == InputEvent::EV_KEY_UP)
						inputStates_[a] = 0.f;
				}
			}
			break;
			case bindingDescriptor::DeviceType::Gamepad: {
				// TODO gamepad support
			}
			break;
			case bindingDescriptor::DeviceType::Mouse: {
				if (bindings[a].analog) {
					// treat mouse movement
					if (ev.type == InputEvent::EV_MOUSE_MOVED || ev.type == InputEvent::EV_MOUSE_SCROLL) {
						if (bindings[a].code == 0) 		// X-axis
							inputStates_[a] += ev.dx * mouseSensitivity;
						else if (bindings[a].code == 1) // Y-axis
							inputStates_[a] += ev.dy * mouseSensitivity * (invertMouseY ? -1.f : 1.f);
						else if (bindings[a].code == 2) // Z-axis
							inputStates_[a] += ev.dz;	// we don't apply sensitivity to scroll wheel
					}
				} else if (ev.mouseButton == bindings[a].code) {
					// treat mouse buttons
					if (ev.type == InputEvent::EV_MOUSE_DOWN)
						inputStates_[a] = 1.f;
					else if (ev.type == InputEvent::EV_MOUSE_UP)
						inputStates_[a] = 0.f;
				}
			}
		}
	}
}

void PlayerInputHandler::update(float dt) {
	auto targetSP = targetObj_.lock();
	if (!targetSP) {
		targetObj_.reset();
		return;		// our target object was destroyed
	}
	if (inputStates_[MOVE_FORWARD])
		targetSP->move(IUserControllable::FORWARD);
	if (inputStates_[MOVE_BACKWARD])
		targetSP->move(IUserControllable::BACKWARD);
	if (inputStates_[MOVE_LEFT])
		targetSP->move(IUserControllable::LEFT);
	if (inputStates_[MOVE_RIGHT])
		targetSP->move(IUserControllable::RIGHT);
	if (inputStates_[MOVE_UP])
		targetSP->move(IUserControllable::UP);
	if (inputStates_[MOVE_DOWN])
		targetSP->move(IUserControllable::DOWN);
		
	targetSP->toggleRun(inputStates_[RUN]);
	
	if (inputStates_[ROTATE_YAW]) {
		targetSP->rotate(IUserControllable::RIGHT, inputStates_[ROTATE_YAW] * dt);	// rotating RIGHT with negative angle rotates LEFT
		inputStates_[ROTATE_YAW] = 0;
	}
	if (inputStates_[ROTATE_PITCH]) {
		targetSP->rotate(IUserControllable::DOWN, inputStates_[ROTATE_PITCH] * dt);	// rotating DOWN with negative angle rotates UP
		inputStates_[ROTATE_PITCH] = 0;
	}
	
	for (int a=CUSTOM_ACTION_1; a < CUSTOM_ACTION_LAST; a++)
		targetSP->setActionState(a - CUSTOM_ACTION_1, inputStates_[a]);
}
