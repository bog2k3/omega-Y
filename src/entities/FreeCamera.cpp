#include "FreeCamera.h"
#include <boglfw/math/aabb.h>
#include <boglfw/math/math3D.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

FreeCamera::FreeCamera(glm::vec3 position, glm::vec3 direction)
{
	transform_.setPosition(position);
	glm::vec3 up {0.f, 1.f, 0.f};
	glm::mat4 mRot = buildMatrixFromOrientation(glm::vec3{0.f}, direction, up);
	transform_.setOrientation(glm::quat(mRot));
}

FreeCamera::~FreeCamera()
{
}

void FreeCamera::move(direction dir) {
	switch (dir) {
		case IUserControllable::FORWARD:
			frameMoveValues_.z += 1.f; break;
		case IUserControllable::BACKWARD:
			frameMoveValues_.z -= 1.f; break;
		case IUserControllable::LEFT:
			frameMoveValues_.x -= 1.f; break;
		case IUserControllable::RIGHT:
			frameMoveValues_.x += 1.f; break;
		case IUserControllable::UP:
			frameMoveValues_.y += 1.f; break;
		case IUserControllable::DOWN:
			frameMoveValues_.y -= 1.f; break;
		default:
			break;
	}
}

void FreeCamera::rotate(direction dir, float angle) {
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

void FreeCamera::update(float dt) {
	const float maxMoveSpeed = 3.f * (running_ ? 8.f : 1.f); // m/s
	const float linearAcceleration = maxMoveSpeed / 0.25f;	// m/s^2	- we want to reach the target speed in 0.25 seconds

	// compute the speed alteration based on inputs
	float fmv_len = glm::length(frameMoveValues_);
	if (fmv_len > 0.f)
		frameMoveValues_ /= fmv_len;	// normalize direction vector
	frameMoveValues_ *= maxMoveSpeed;	// this vector now represents our target speed in camera space
	// transform it into world space:
	frameMoveValues_ = transform_.orientation() * frameMoveValues_;
	// how much ground we have to cover to reach that speed
	glm::vec3 delta = frameMoveValues_ - speed_;
	float factor = clamp(linearAcceleration * dt, 0.f, 1.f);
	speed_ += delta * factor;
	transform_.moveWorld(speed_ * dt);
	frameMoveValues_ = glm::vec3(0.f);

	// compute rotation alteration based on inputs
	auto deltaRot = frameRotateValues_;
	transform_.rotateWorld(glm::quat(glm::vec3(deltaRot.x, 0.f, 0.f)));
	transform_.rotateLocal(glm::quat(glm::vec3(0.f, deltaRot.y, 0.f)));
	frameRotateValues_ = glm::vec3(0.f);
}
