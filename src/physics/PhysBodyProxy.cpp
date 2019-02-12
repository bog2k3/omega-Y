#include "PhysBodyProxy.h"
#include "math.h"

#include <boglfw/World.h>

#include <bullet3/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet3/LinearMath/btDefaultMotionState.h>
#include <bullet3/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <bullet3/BulletCollision/CollisionShapes/btCollisionShape.h>

#include <glm/gtc/quaternion.hpp>

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
	bodyPtr->setLinearVelocity(g2b(cfg.initialVelocity));
	bodyPtr->setAngularVelocity(g2b(glm::eulerAngles(cfg.initialAngularVelocity)));
	World::getGlobal<btDiscreteDynamicsWorld>()->addRigidBody(bodyPtr);

	collisionShape_ = cfg.shape;
}

void PhysBodyProxy::updateTransform(Transform &tr) {
	// update transform from physics:
	btTransform wTrans;
	motionState->getWorldTransform(wTrans);	// we get the interpolated transform here
	tr.setPosition(b2g(wTrans.getOrigin()));
	tr.setOrientation(b2g(wTrans.getRotation()));
}
