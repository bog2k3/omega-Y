#include "PlayerEntity.h"
#include "../physics/math.h"

#include <boglfw/math/math3D.h>
#include <boglfw/math/aabb.h>
#include <boglfw/World.h>

#include <bullet3/BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <bullet3/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet3/LinearMath/btDefaultMotionState.h>
#include <bullet3/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>

#include <glm/gtc/quaternion.hpp>

PlayerEntity::PlayerEntity(glm::vec3 position, float heading)
	: physicsBodyMeta_(this, EntityTypes::PLAYER)
{
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	transform_.setPosition(position);
	// create player shape
	physicsShape_ = new btCapsuleShape{0.3f, 1.5f};
	btVector3 inertia;
	float mass = 70.f;
	physicsShape_->calculateLocalInertia(mass, inertia);

	// create player body
	btVector3 vPos = g2b(position);
	btQuaternion qOrient(heading, 0.f, 0.f);
	physMotionState_ = new btDefaultMotionState(btTransform{qOrient, vPos});
	btRigidBody::btRigidBodyConstructionInfo cinfo {
		mass,
		physMotionState_,
		physicsShape_,
		inertia
	};
	cinfo.m_friction = 0.5f;

	physicsBodyMeta_.bodyPtr = new btRigidBody(cinfo);
	physicsBodyMeta_.bodyPtr->setUserPointer(&physicsBodyMeta_);
	physicsBodyMeta_.bodyPtr->setAngularFactor(0.f);
	physicsBodyMeta_.bodyPtr->setSleepingThresholds(0.f, 0.f);	// prevent player body from falling asleep
	World::getGlobal<btDiscreteDynamicsWorld>()->addRigidBody(physicsBodyMeta_.bodyPtr);

	physicsBodyMeta_.collisionCfg[EntityTypes::TERRAIN] = true;
	physicsBodyMeta_.onCollision.add(std::bind(&PlayerEntity::onCollision, this, std::placeholders::_1));
}

PlayerEntity::~PlayerEntity()
{
	World::getGlobal<btDiscreteDynamicsWorld>()->removeRigidBody(physicsBodyMeta_.bodyPtr);
	delete physicsBodyMeta_.bodyPtr, physicsBodyMeta_.bodyPtr = nullptr;
	delete physicsShape_, physicsShape_ = nullptr;
	delete physMotionState_, physMotionState_ = nullptr;
}

void PlayerEntity::draw(Viewport* vp) {

}

void PlayerEntity::moveTo(glm::vec3 where) {
	physicsBodyMeta_.bodyPtr->setWorldTransform(btTransform{btQuaternion::getIdentity(), g2b(where)});
	physicsBodyMeta_.bodyPtr->activate();
}

void PlayerEntity::update(float dt) {
	canJump_ = false;

	// update transform from physics:
	btTransform wTrans;
	physMotionState_->getWorldTransform(wTrans);	// we get the interpolated transform here
	transform_.setPosition(b2g(wTrans.getOrigin()));
	transform_.setOrientation(b2g(wTrans.getRotation()));

	// compute movement based on inputs
	const float moveSpeed = 1.5f * (running_ ? 4.f : 1.f); // m/s
	const float jumpSpeed = 2.f; // m/s

	float len = glm::length(frameMoveValues_);
	glm::vec2 vDir = len > 0 ? frameMoveValues_ / len : glm::vec2{0.f, 0.f};
	float verticalSpeed = physicsBodyMeta_.bodyPtr->getLinearVelocity().getY();
	if (jump_)
		verticalSpeed += jumpSpeed, jump_ = false;
	glm::vec3 vSpeed = {vDir.x * moveSpeed, 0.f, vDir.y * moveSpeed};
	// transform speed in world space
	vSpeed = transform_.orientation() * vSpeed;
	vSpeed.y = verticalSpeed;
	// apply it to the body:
	physicsBodyMeta_.bodyPtr->setLinearVelocity(g2b(vSpeed));
	frameMoveValues_ = glm::vec2{0.f};

	// compute rotation alteration based on inputs
	btTransform realTr = physicsBodyMeta_.bodyPtr->getWorldTransform();	// we need to operate on the 'real' (non-interpolated) transform here
	auto lRot = glm::quat(glm::vec3(0.f, frameRotateValues_.y, 0.f));
	auto wRot = glm::quat(glm::vec3(frameRotateValues_.x, 0.f, 0.f));
	auto rot = b2g(realTr.getRotation());
	rot = lRot * rot * wRot;
	realTr.setRotation(g2b(rot));
	physicsBodyMeta_.bodyPtr->setWorldTransform(realTr);
	frameRotateValues_ = glm::vec2{0.f};
}

void PlayerEntity::move(direction dir) {
	switch (dir) {
		case IUserControllable::FORWARD:
			frameMoveValues_.y += 1.f;
		break;
		case IUserControllable::BACKWARD:
			frameMoveValues_.y -= 1.f;
		break;
		case IUserControllable::LEFT:
			frameMoveValues_.x -= 1.f;
		break;
		case IUserControllable::RIGHT:
			frameMoveValues_.x += 1.f;
		break;
		case IUserControllable::UP:
			if (canJump_)
				jump_ = true, canJump_ = false;
		break;
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
	switch (actionId) {
	case ACTION_SHOOT:
		setWeaponTriggerState(on);
	break;
	case ACTION_RELOAD:
		setWeaponReloadState(on);
	break;
	default:
		break;
	}
}

void PlayerEntity::onCollision(CollisionEvent const& ev) {
	switch (ev.pOtherMeta->entityType) {
		case EntityTypes::TERRAIN:
			canJump_ = true;
			break;
	}
}

#ifdef DEBUG
void PlayerEntity::testShootBullet() {

}
#endif // DEBUG

void PlayerEntity::setWeaponTriggerState(bool on) {
	// TODO use on/off to implement continuous shooting
	if (on) {
#ifdef DEBUG
		// one shot action
		testShootBullet();
#endif // DEBUG
	}
}

void PlayerEntity::setWeaponReloadState(bool on) {
	// TODO use on/off to implement long reloads for some weapons
	if (on) {
		// ... reload weapon
	}
}
