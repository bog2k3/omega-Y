#include "PlayerInputHandler.h"

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
	bindings[MOVE_UP] 			= { InputEvent::MB_RIGHT, false, bindingDescriptor::DeviceType::Mouse };
	bindings[MOVE_DOWN] 		= { InputEvent::MB_LEFT, false, bindingDescriptor::DeviceType::Mouse };
	bindings[ROTATE_YAW] 		= { 0, true, bindingDescriptor::DeviceType::Mouse };	// mouse x axis
	bindings[ROTATE_PITCH] 		= { 1, true, bindingDescriptor::DeviceType::Mouse };	// mouse y axis
}

void PlayerInputHandler::handleInputEvent(InputEvent& ev) {
	// for now we don't have a player, just a free camera
	
}

void PlayerInputHandler::update(float dt) {

}
