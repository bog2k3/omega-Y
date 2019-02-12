#include "PlayerEntity.h"
#include "../physics/math.h"
#include "weapons/Pistol.h"
#include "weapons/TestBullet.h"

#include <boglfw/math/math3D.h>
#include <boglfw/math/aabb.h>
#include <boglfw/World.h>

#include <bullet3/BulletCollision/CollisionShapes/btCapsuleShape.h>
#include <bullet3/BulletDynamics/Dynamics/btRigidBody.h>
#include <bullet3/LinearMath/btDefaultMotionState.h>

#include <glm/gtc/quaternion.hpp>

PlayerEntity::PlayerEntity(glm::vec3 position, float heading)
	: physicsBodyProxy_(this)
{
#ifdef DEBUG
	World::assertOnMainThread();
#endif
	transform_.setPosition(position);
	transform_.setOrientation(glm::quat(glm::vec3{heading, 0.f, 0.f}));

	// create player body
	PhysBodyConfig bodyCfg;
	bodyCfg.mass = 70.f;
	bodyCfg.position = position;
	bodyCfg.orientation = transform_.orientation();
	bodyCfg.friction = 0.5f;
	bodyCfg.shape = std::make_shared<btCapsuleShape>(0.3f, 1.5f);

	physicsBodyProxy_.createBody(bodyCfg);

	physicsBodyProxy_.bodyPtr->setAngularFactor(0.f);
	physicsBodyProxy_.bodyPtr->setSleepingThresholds(0.f, 0.f);	// prevent player body from falling asleep

	physicsBodyProxy_.collisionCfg[EntityTypes::TERRAIN] = true;
	physicsBodyProxy_.onCollision.add(std::bind(&PlayerEntity::onCollision, this, std::placeholders::_1));

	pWeapon_ = new Pistol();
	pWeapon_->feedAmmo(100);
	pWeapon_->forceReload(pWeapon_->getMagazineSize());
}

PlayerEntity::~PlayerEntity() {
}

void PlayerEntity::draw(Viewport* vp) {
	pWeapon_->draw(transform_);
}

void PlayerEntity::moveTo(glm::vec3 where) {
	physicsBodyProxy_.bodyPtr->setWorldTransform(btTransform{btQuaternion::getIdentity(), g2b(where)});
	physicsBodyProxy_.bodyPtr->activate();
}

void PlayerEntity::update(float dt) {
	canJump_ = false;

	physicsBodyProxy_.updateTransform(transform_);

	// compute movement based on inputs
	const float moveSpeed = 1.5f * (running_ ? 4.f : 1.f); // m/s
	const float jumpSpeed = 2.f; // m/s

	float len = glm::length(frameMoveValues_);
	glm::vec2 vDir = len > 0 ? frameMoveValues_ / len : glm::vec2{0.f, 0.f};
	float verticalSpeed = physicsBodyProxy_.bodyPtr->getLinearVelocity().getY();
	if (jump_)
		verticalSpeed += jumpSpeed, jump_ = false;
	glm::vec3 vSpeed = {vDir.x * moveSpeed, 0.f, vDir.y * moveSpeed};
	// transform speed in world space
	vSpeed = transform_.orientation() * vSpeed;
	vSpeed.y = verticalSpeed;
	// apply it to the body:
	physicsBodyProxy_.bodyPtr->setLinearVelocity(g2b(vSpeed));
	frameMoveValues_ = glm::vec2{0.f};

	// compute rotation alteration based on inputs
	btTransform realTr = physicsBodyProxy_.bodyPtr->getWorldTransform();	// we need to operate on the 'real' (non-interpolated) transform here
	auto lRot = glm::quat(glm::vec3(0.f, frameRotateValues_.y, 0.f));
	auto wRot = glm::quat(glm::vec3(frameRotateValues_.x, 0.f, 0.f));
	auto rot = b2g(realTr.getRotation());
	rot = lRot * rot * wRot;
	realTr.setRotation(g2b(rot));
	physicsBodyProxy_.bodyPtr->setWorldTransform(realTr);
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
	// shoot a test projectile
	glm::vec3 offset {0.f, 0.f, 0.5f};
	glm::vec3 pos = transform_.position() + transform_.orientation() * offset;
	glm::quat orient = transform_.orientation();
	float bulletSpeed = 10.f;
	glm::vec3 velocity = transform_.orientation() * glm::vec3{0.f, 0.f, bulletSpeed};
	std::shared_ptr<TestBullet> pb = std::make_shared<TestBullet>(pos, orient, velocity, glm::quat{1.f, 0.f, 0.f, 0.f});
	World::getInstance().takeOwnershipOf(pb);
}
#endif // DEBUG

void PlayerEntity::setWeaponTriggerState(bool on) {
	pWeapon_->toggleTrigger(true, on);
	if (on) {
#ifdef DEBUG
		// one shot action
		testShootBullet();
#endif // DEBUG
	}
}

void PlayerEntity::setWeaponReloadState(bool on) {
	pWeapon_->toggleReload(on);
}
