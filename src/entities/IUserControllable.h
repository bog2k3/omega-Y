#ifndef __I_USER_CONTROLLABLE_H__
#define __I_USER_CONTROLLABLE_H__

// interface for user controllable objects, such as player and free-camera.
// provides methods for moving around and triggering indexed actions
class IUserControllable {
public:
	virtual ~IUserControllable() = default;
		
	enum direction {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		UP,
		DOWN
	};
	
	virtual void move(direction dir) = 0;
	virtual void toggleRun(bool on) = 0;
	virtual void rotate(direction dir, float angle) = 0;
	
	// sets an action state to ON or OFF as the user presses or releases the associated key/button.
	// the meaning of the action is defined by the implementation
	virtual void setActionState(int actionId, bool on) = 0;
};

#endif // __I_USER_CONTROLLABLE_H__