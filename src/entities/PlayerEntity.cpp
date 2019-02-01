#include "PlayerEntity.h"
#include "../physics/math.h"

#include <boglfw/math/math3D.h>
#include <boglfw/math/aabb.h>
#include <boglfw/World.h>

#include <bullet3/BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <bullet3/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet3/LinearMath/btDefaultMotionState.h>
#include <bullet3/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

#include <bullet3/BulletDynamics/Character/btKinematicCharacterController.h>

#include <glm/gtc/quaternion.hpp>

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
	physMotionState_ = new btDefaultMotionState(btTransform{qOrient, vPos});
	btRigidBody::btRigidBodyConstructionInfo cinfo {
		mass,
		physMotionState_,
		physicsShape_,
		inertia
	};
	cinfo.m_friction = 0.5f;

	physicsBody_ = new btRigidBody(cinfo);
	physicsBody_->setAngularFactor(0.f);
	physicsBody_->setSleepingThresholds(0.f, 0.f);	// prevent player body from falling asleep
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

void PlayerEntity::moveTo(glm::vec3 where) {
	physicsBody_->setWorldTransform(btTransform{btQuaternion::getIdentity(), g2b(where)});
	physicsBody_->activate();
}

void PlayerEntity::update(float dt) {
	// update transform from physics:
	btTransform wTrans;
	physMotionState_->getWorldTransform(wTrans);
	transform_.setPosition(b2g(wTrans.getOrigin()));
	transform_.setOrientation(b2g(wTrans.getRotation()));

	// compute movement based on inputs
	const float moveSpeed = 2.f * (running_ ? 2.f : 1.f); // m/s
	const float jumpSpeed = 2.5f; // m/s

	float len = glm::length(frameMoveValues_);
	glm::vec2 vDir = len > 0 ? frameMoveValues_ / len : glm::vec2{0.f, 0.f};
	float verticalSpeed = physicsBody_->getLinearVelocity().getY();
	if (jump_)
		verticalSpeed += jumpSpeed, jump_ = false;
	glm::vec3 vSpeed = {vDir.x * moveSpeed, verticalSpeed, vDir.y * moveSpeed};
	// transform speed in world space
	vSpeed = transform_.orientation() * vSpeed;
	// apply it to the body:
	physicsBody_->setLinearVelocity(g2b(vSpeed));
	frameMoveValues_ = glm::vec2{0.f};

	// compute rotation alteration based on inputs
	auto tr = physicsBody_->getWorldTransform();
	auto lRot = glm::quat(glm::vec3(frameRotateValues_.x, frameRotateValues_.y, 0.f));
	//auto wRot = glm::transfo crtTransf * lRot;
	auto rot = tr.getRotation();
	//rot *= lRot;
	tr.setRotation(rot);
	physicsBody_->setWorldTransform(tr);
	frameRotateValues_ = glm::vec2{0.f};
}

void PlayerEntity::move(direction dir) {
	switch (dir) {
		case IUserControllable::FORWARD:
			frameMoveValues_.y += 1.f; break;
		case IUserControllable::BACKWARD:
			frameMoveValues_.y -= 1.f; break;
		case IUserControllable::LEFT:
			frameMoveValues_.x -= 1.f; break;
		case IUserControllable::RIGHT:
			frameMoveValues_.x += 1.f; break;
		case IUserControllable::UP:
			jump_ = true; break;
		case IUserControllable::DOWN:
			break;
		default:
			break;
	}
}

void PlayerEntity::toggleRun(bool on) {
	running_ = on;
}

void PlayerEntity::rotate(direction dir, float angle) {
	switch (dir) {
		case IUserControllable::LEFT:
			frameRotateValues_.y -= angle; break;
		case IUserControllable::RIGHT:
			frameRotateValues_.y += angle; break;
		case IUserControllable::UP:
			frameRotateValues_.x -= angle; break;
		case IUserControllable::DOWN:
			frameRotateValues_.x += angle; break;
		default:
			break;
	}
}

void PlayerEntity::setActionState(int actionId, bool on) {

}
