#include "PhysBodyProxy.h"
#include "math.h"

#include <boglfw/World.h>

#include <bullet3/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet3/LinearMath/btDefaultMotionState.h>
#include <bullet3/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <bullet3/BulletCollision/CollisionShapes/btCollisionShape.h>

PhysBodyProxy::~PhysBodyProxy() {
	reset();
}

void PhysBodyProxy::reset() {
	if (bodyPtr) {
		World::getGlobal<btDiscreteDynamicsWorld>()->removeRigidBody(bodyPtr);
		delete bodyPtr, bodyPtr = nullptr;
		if (motionState)
			delete motionState, motionState = nullptr;
		collisionShape_.reset();
	}
}

void PhysBodyProxy::createBody(PhysBodyConfig const& cfg) {
	assert(cfg.shape != nullptr);
	assert(cfg.mass >= 0.f);
	btVector3 inertia;
	cfg.shape->calculateLocalInertia(cfg.mass, inertia);

	btVector3 vPos = g2b(cfg.position);
	btQuaternion qOrient = g2b(cfg.orientation);
	motionState = new btDefaultMotionState(btTransform{qOrient, vPos});
	btRigidBody::btRigidBodyConstructionInfo cinfo {
		cfg.mass,
		motionState,
		cfg.shape.get(),
		inertia
	};
	cinfo.m_friction = cfg.friction;

	bodyPtr = new btRigidBody(cinfo);
	bodyPtr->setUserPointer(this);
	World::getGlobal<btDiscreteDynamicsWorld>()->addRigidBody(bodyPtr);

	collisionShape_ = cfg.shape;
}
