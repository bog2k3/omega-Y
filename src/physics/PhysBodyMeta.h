#ifndef __PHYS_BODY_META_H__
#define __PHYS_BODY_META_H__

#include <boglfw/utils/Event.h>

#include <glm/vec3.hpp>

#include <unordered_map>

class Entity;
class btRigidBody;
class btCollisionObject;
class PhysBodyMeta;

struct CollisionEvent {
	const btCollisionObject* pThisObj;
	const btCollisionObject* pOtherObj;
	PhysBodyMeta* pThisMeta;
	PhysBodyMeta* pOtherMeta;
	enum : int { maxNumberContacts = 4 };
	struct ContactPoint {
		glm::vec3 worldPointOnThis;
		glm::vec3 worldPointOnOther;
		//glm::vec3 worldNormalOnOther;
	} contacts[maxNumberContacts];
	int numContacts;
};

// every btCollisionObject in the world has a userPointer to this struct:
struct PhysBodyMeta {
	PhysBodyMeta(Entity* entityPtr, unsigned entityType)
		: entityType(entityType), entityPtr(entityPtr) {
	}

	const unsigned entityType;		// type of associated entity
	const Entity* entityPtr;		// pointer to associated entity
	btRigidBody* bodyPtr = nullptr;	// pointer to the rigid body (or nullptr if the btCollisionObject is not a btRigidBody)

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
};

#endif // __PHYS_BODY_META_H__
