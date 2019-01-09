#ifndef PLAYERINPUTHANDLER_H
#define PLAYERINPUTHANDLER_H

class InputEvent;

class PlayerInputHandler
{
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
		ROTATE_YAW,
		ROTATE_PITCH,

		ALL_ACTIONS
	};

	struct bindingDescriptor {
		int code;	// keycode or button id or axis id, depending on device and analog option
		bool analog = false;	// if true, code refers to an analog axis instead of a button
		enum class DeviceType {
			Keyboard,
			Mouse,
			Gamepad
		} device;
	};

	bindingDescriptor bindings[ALL_ACTIONS];	// maps each action to a key/button/analog axis

	void setDefaultBindings();

	void handleInputEvent(InputEvent& ev);
	void update(float dt);

};

#endif // PLAYERINPUTHANDLER_H
