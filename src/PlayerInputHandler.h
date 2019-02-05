#ifndef PLAYERINPUTHANDLER_H
#define PLAYERINPUTHANDLER_H

#include <memory>

class InputEvent;
class IUserControllable;

class PlayerInputHandler {
public:
	PlayerInputHandler();
	~PlayerInputHandler();

	enum actions {
		MOVE_FORWARD = 0,
		MOVE_BACKWARD,
		MOVE_LEFT,
		MOVE_RIGHT,
		MOVE_UP,
		MOVE_DOWN,
		RUN,
		ROTATE_YAW,
		ROTATE_PITCH,

		CUSTOM_ACTION_1,
		CUSTOM_ACTION_2,
		CUSTOM_ACTION_3,
		CUSTOM_ACTION_4,
		CUSTOM_ACTION_5,

		CUSTOM_ACTION_LAST,

		ALL_ACTIONS
	};

	struct bindingDescriptor {
		int code = 0;	// keycode or button id or axis id, depending on device and analog option
		bool analog = false;	// if true, code refers to an analog axis instead of a button
		enum class DeviceType {
			None,
			Keyboard,
			Mouse,
			Gamepad
		} device = DeviceType::None;
	};

	bindingDescriptor bindings[ALL_ACTIONS];	// maps each action to a key/button/analog axis
	float mouseSensitivity = 0.2f;				// radians per hundred pixels
	bool invertMouseY = false;					// invert mouse Y axis

	void setDefaultBindings();

	void handleInputEvent(InputEvent& ev);
	void update(float dt);

	void setTargetObject(std::weak_ptr<IUserControllable> target) { targetObj_ = target; }

private:
	float inputStates_[ALL_ACTIONS];				// input button/axis states (values) for each bound input
	std::weak_ptr<IUserControllable> targetObj_;	// this is the target to which actions are routed
};

#endif // PLAYERINPUTHANDLER_H
