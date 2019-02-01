#ifndef PLAYERENTITY_H
#define PLAYERENTITY_H

#include "enttypes.h"
#include "IUserControllable.h"
#include <boglfw/entities/Entity.h>

class btCapsuleShape;
class btRigidBody;
class btMotionState;

class PlayerEntity : public Entity, public IUserControllable
{
public:
	PlayerEntity(glm::vec3 position, glm::vec3 direction);
	virtual ~PlayerEntity() override;
	virtual FunctionalityFlags getFunctionalityFlags() const override { return FunctionalityFlags::DRAWABLE | FunctionalityFlags::UPDATABLE; }
	virtual unsigned getEntityType() const override { return EntityTypes::PLAYER; }

	virtual void update(float dt) override;
	virtual void draw(Viewport* vp) override;

	virtual void move(direction dir) override;
	virtual void toggleRun(bool on) override;
	virtual void rotate(direction dir, float angle) override;

	void moveTo(glm::vec3 where);

	// sets an action state to ON or OFF as the user presses or releases the associated key/button.
	// the meaning of the action is defined by the implementation
	virtual void setActionState(int actionId, bool on) override;

private:
	//Mesh mesh_;

	btRigidBody* physicsBody_ = nullptr;
	btCapsuleShape* physicsShape_ = nullptr;
	btMotionState* physMotionState_ = nullptr;

	bool running_ = false;
	bool jump_ = false;
	glm::vec2 frameMoveValues_ {0.f};
	glm::vec2 frameRotateValues_ {0.f};

	//glm::vec3 speed_;
};

#endif // PLAYERENTITY_H
