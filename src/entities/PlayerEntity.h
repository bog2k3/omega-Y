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
	virtual aabb getAABB(bool requirePrecise=false) const override;
	virtual glm::mat4 getTransform() const override;

	virtual void move(direction dir) override;
	virtual void toggleRun(bool on) override;
	virtual void rotate(direction dir, float angle) override;

	void moveTo(glm::vec3 where);

	// sets an action state to ON or OFF as the user presses or releases the associated key/button.
	// the meaning of the action is defined by the implementation
	virtual void setActionState(int actionId, bool on) override;

private:
	//Mesh mesh_;
	//glm::vec3 position_;
	//glm::vec3 direction_;
	//mutable glm::mat4 transform_;
	//mutable bool transformDirty_ = true;

	btRigidBody* physicsBody_ = nullptr;
	btCapsuleShape* physicsShape_ = nullptr;
	btMotionState* physMotionState_ = nullptr;

	//glm::vec3 speed_;
};

#endif // PLAYERENTITY_H
