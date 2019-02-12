#ifndef __PHYS_BODY_META_H__
#define __PHYS_BODY_META_H__

#include <boglfw/entities/Entity.h>
#include <boglfw/utils/Event.h>

#include <glm/vec3.hpp>

#include <unordered_map>
#include <memory>

class btRigidBody;
class btCollisionShape;
class btCollisionObject;
class btDefaultMotionState;
class PhysBodyProxy;

struct CollisionEvent {
	const btCollisionObject* pThisObj;
	const btCollisionObject* pOtherObj;
	PhysBodyProxy* pThisMeta;
	PhysBodyProxy* pOtherMeta;
	enum : int { maxNumberContacts = 4 };
	struct ContactPoint {
		glm::vec3 worldPointOnThis;
		glm::vec3 worldPointOnOther;
		//glm::vec3 worldNormalOnOther;
	} contacts[maxNumberContacts];
	int numContacts;
};

// construction config for physics body
struct PhysBodyConfig {
	std::shared_ptr<btCollisionShape> shape;
	float mass = 1.f;
	float friction = 0.5f;
	glm::vec3 position {0.f};
	glm::quat orientation {1.f, 0.f, 0.f, 0.f};
	// initial velocity in m/s expressed in world space
	glm::vec3 initialVelocity {0.f};
	// initial angular velocity in rad/s expressed in world space
	glm::quat initialAngularVelocity {1.f, 0.f, 0.f, 0.f};
};

// every btCollisionObject in the world has a userPointer to this class:
class PhysBodyProxy {
public:
	PhysBodyProxy(Entity* entityPtr)
		: entityType(entityPtr ? entityPtr->getEntityType() : 0)
		, entityPtr(entityPtr) {
	}

	// destructor calls reset() automatically
	~PhysBodyProxy();

	// This will remove the body from the physics world,
	// delete the body, delete the motion_state and release the shape.
	void reset();

	// helper function to create the physics body.
	// This will automatically add the body to the physics world.
	void createBody(PhysBodyConfig const& cfg);

	// updates the given transform from the physics body's interpolated transform.
	void updateTransform(Transform &tr);

	const unsigned entityType;		// type of associated entity
	const Entity* entityPtr;		// pointer to associated entity
	btRigidBody* bodyPtr = nullptr;	// pointer to the rigid body (or nullptr if the btCollisionObject is not a btRigidBody)
	btDefaultMotionState* motionState = nullptr; // body's motion state

	// enables generating collision events of the associated body against other entities bodies
	// key is entity type, value is true/false to enable/disable generating event against that entity type.
	// For other entity types that are not configured here, the default behaviour is to not generate any event.
	std::unordered_map<unsigned, bool> collisionCfg;

	// this event is triggered when the associated body collides with another body.
	// the event is only triggered for those other entity types that have been configured
	// via collisionCfg (where generateEvent is true).
	// The event is deferred until all physics have finished updating, then all events are being emitted in a separate step.
	// The event is *ALWAYS* emitted on the main thread.
	Event<void(CollisionEvent const& ev)> onCollision;

private:
	std::shared_ptr<btCollisionShape> collisionShape_;
};

#endif // __PHYS_BODY_META_H__
