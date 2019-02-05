#ifndef PLAYERENTITY_H
#define PLAYERENTITY_H

#include "enttypes.h"
#include "IUserControllable.h"
#include "../physics/PhysBodyMeta.h"
#include <boglfw/entities/Entity.h>

class btCapsuleShape;
class btRigidBody;
class btMotionState;

class PlayerEntity : public Entity, public IUserControllable {
public:
	// heading is measured CCW from +Z axis (0.0 means looking down +Z)
	PlayerEntity(glm::vec3 position, float heading);
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

	PhysBodyMeta physicsBodyMeta_;
	btCapsuleShape* physicsShape_ = nullptr;
	btMotionState* physMotionState_ = nullptr;

	bool running_ = false;
	bool jump_ = false;
	bool canJump_ = false;
	glm::vec2 frameMoveValues_ {0.f};
	glm::vec2 frameRotateValues_ {0.f};

	void onCollision(CollisionEvent const& ev);
};

#endif // PLAYERENTITY_H
