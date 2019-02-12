#include "PlayerInputHandler.h"
#include "entities/IUserControllable.h"

#include <boglfw/input/InputEvent.h>
#include <boglfw/utils/log.h>

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
	bindings[MOVE_DOWN] 		= { GLFW_KEY_LEFT_CONTROL, false, bindingDescriptor::DeviceType::Keyboard };
	bindings[ROTATE_YAW] 		= { 0, true, bindingDescriptor::DeviceType::Mouse };	// mouse x axis
	bindings[ROTATE_PITCH] 		= { 1, true, bindingDescriptor::DeviceType::Mouse };	// mouse y axis

	bindings[CUSTOM_ACTION_1] 	= { InputEvent::MB_LEFT, false, bindingDescriptor::DeviceType::Mouse };
}

void PlayerInputHandler::handleInputEvent(InputEvent& ev) {
	for (int a=0; a < ALL_ACTIONS; a++) {
		switch (bindings[a].device) {
			case bindingDescriptor::DeviceType::Keyboard: {
				// treat keyboard buttons
				if (ev.key == bindings[a].code) {
					if (ev.type == InputEvent::EV_KEY_DOWN) {
						inputStates_[a].second = inputStates_[a].first != 1.f;
						inputStates_[a].first = 1.f;					// TODO change to += 1.f if allowing multiple bindings for same action
						ev.consume();
					} else if (ev.type == InputEvent::EV_KEY_UP) {
						inputStates_[a].second = inputStates_[a].first != 0.f;
						inputStates_[a].first = 0.f;
						ev.consume();
					}
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
						if (bindings[a].code == 0) { 		// X-axis
							inputStates_[a].first += ev.dx * mouseSensitivity * 0.01f;
							inputStates_[a].second = true;
							ev.consume();
						}
						else if (bindings[a].code == 1) { // Y-axis
							inputStates_[a].first += ev.dy * mouseSensitivity * 0.01f * (invertMouseY ? -1.f : 1.f);
							inputStates_[a].second = true;
							ev.consume();
						}
						else if (bindings[a].code == 2) { // Z-axis
							inputStates_[a].first += ev.dz;	// we don't apply sensitivity to scroll wheel
							inputStates_[a].second = true;
							ev.consume();
						}
					}
				} else if (ev.mouseButton == bindings[a].code) {
					// treat mouse buttons
					if (ev.type == InputEvent::EV_MOUSE_DOWN) {
						inputStates_[a].second = inputStates_[a].first != 1.f;
						inputStates_[a].first = 1.f;
						ev.consume();
					}
					else if (ev.type == InputEvent::EV_MOUSE_UP) {
						inputStates_[a].second = inputStates_[a].first != 0.f;
						inputStates_[a].first = 0.f;
						ev.consume();
					}
				}
			} break;
			case bindingDescriptor::DeviceType::None:
				// no device bound to this action
			break;
			default:
				ERROR("PlayerInputHandler: invalid device type " << (int)bindings[a].device);
				break;
		}
	}
}

void PlayerInputHandler::update(float dt) {
	auto targetSP = targetObj_.lock();
	if (!targetSP) {
		targetObj_.reset();
		return;		// our target object was destroyed
	}
	if (inputStates_[MOVE_FORWARD].first)
		targetSP->move(IUserControllable::FORWARD);
	if (inputStates_[MOVE_BACKWARD].first)
		targetSP->move(IUserControllable::BACKWARD);
	if (inputStates_[MOVE_LEFT].first)
		targetSP->move(IUserControllable::LEFT);
	if (inputStates_[MOVE_RIGHT].first)
		targetSP->move(IUserControllable::RIGHT);
	if (inputStates_[MOVE_UP].first)
		targetSP->move(IUserControllable::UP);
	if (inputStates_[MOVE_DOWN].first)
		targetSP->move(IUserControllable::DOWN);

	if (inputStates_[RUN].second) {
		targetSP->toggleRun(inputStates_[RUN].first);
		inputStates_[RUN].second = false;
	}

	if (inputStates_[ROTATE_YAW].first) {
		targetSP->rotate(IUserControllable::RIGHT, inputStates_[ROTATE_YAW].first);	// rotating RIGHT with negative angle rotates LEFT
		inputStates_[ROTATE_YAW].first = 0;
	}
	if (inputStates_[ROTATE_PITCH].first) {
		targetSP->rotate(IUserControllable::DOWN, inputStates_[ROTATE_PITCH].first);	// rotating DOWN with negative angle rotates UP
		inputStates_[ROTATE_PITCH].first = 0;
	}

	for (int a=CUSTOM_ACTION_1; a < CUSTOM_ACTION_LAST; a++) {
		if (inputStates_[a].second) {
			targetSP->setActionState(a - CUSTOM_ACTION_1, inputStates_[a].first);
			inputStates_[a].second = false;
		}
	}
}
