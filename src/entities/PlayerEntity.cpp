#include "PlayerEntity.h"
#include "../physics/math.h"

#include <boglfw/math/math3D.h>
#include <boglfw/math/aabb.h>
#include <boglfw/World.h>

#include <bullet3/BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <bullet3/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet3/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

PlayerEntity::PlayerEntity(glm::vec3 position, glm::vec3 direction)
{
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	// create player shape
	physicsShape_ = new btCapsuleShape{0.3f, 1.7f};
	btVector3 inertia;
	float mass = 70.f;
	physicsShape_->calculateLocalInertia(mass, inertia);

	// create player body
	btVector3 vPos = g2b(position);
	btQuaternion qOrient = btQuaternion::getIdentity();
	btRigidBody::btRigidBodyConstructionInfo cinfo {
		mass,
		nullptr,
		physicsShape_,
		inertia
	};
	cinfo.m_startWorldTransform = btTransform{qOrient, vPos};
	cinfo.m_friction = 0.5f;

	physicsBody_ = new btRigidBody(cinfo);
	World::getGlobal<btDiscreteDynamicsWorld>()->addRigidBody(physicsBody_);
}

PlayerEntity::~PlayerEntity()
{
	World::getGlobal<btDiscreteDynamicsWorld>()->removeRigidBody(physicsBody_);
	delete physicsBody_, physicsBody_ = nullptr;
	delete physicsShape_, physicsShape_ = nullptr;
}

void PlayerEntity::draw(Viewport* vp) {

}

aabb PlayerEntity::getAABB(bool requirePrecise) const {
	// TODO base this on the model
	//glm::vec3 halfSize {0.5f, 0.5f, 0.5f};
	//return {position_ - halfSize, position_ + halfSize};
	return aabb();
}

glm::mat4 PlayerEntity::getTransform() const {
	glm::mat4 m;
	physicsBody_->getWorldTransform().getOpenGLMatrix(&m[0][0]);
	return m;
}

void PlayerEntity::moveTo(glm::vec3 where) {
	physicsBody_->setWorldTransform(btTransform{btQuaternion::getIdentity(), g2b(where)});
	physicsBody_->activate();
}

void PlayerEntity::update(float dt) {

}

void PlayerEntity::move(direction dir) {

}

void PlayerEntity::toggleRun(bool on) {

}

void PlayerEntity::rotate(direction dir, float angle) {

}

void PlayerEntity::setActionState(int actionId, bool on) {

}
