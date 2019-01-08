#ifndef PLAYERINPUTHANDLER_H
#define PLAYERINPUTHANDLER_H

class InputEvent;

class PlayerInputHandler
{
public:
	PlayerInputHandler();
	~PlayerInputHandler();

	void handleInputEvent(InputEvent& ev);
	void update(float dt);

};

#endif // PLAYERINPUTHANDLER_H
