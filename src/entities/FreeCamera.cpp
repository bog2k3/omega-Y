#include "FreeCamera.h"
#include <boglfw/math/aabb.h>
#include <boglfw/math/math3D.h>

FreeCamera::FreeCamera(glm::vec3 position, glm::vec3 direction)
	: position_(position)
	, direction_(direction)
{
}

FreeCamera::~FreeCamera()
{
}

aabb FreeCamera::getAABB(bool requirePrecise) const {
	glm::vec3 halfSize {0.5f, 0.5f, 0.5f};
	return {position_ - halfSize, position_ + halfSize};
}

glm::mat4 FreeCamera::getTransform() const {
	if (transformDirty_) {
		glm::vec3 up {0.f, 1.f, 0.f};
		transform_ = buildMatrixFromOrientation(position_, direction_, up);
		transformDirty_ = false;
	}
	return transform_;
}

void FreeCamera::move(direction dir) {
	switch (dir) {
		case FORWARD:
			frameMoveValues_.z += running_ ? 1.f : 0.5f; break;
		case BACKWARD:
			frameMoveValues_.z -= running_ ? 1.f : 0.5f; break;
		case LEFT:
			frameMoveValues_.x -= running_ ? 1.f : 0.5f; break;
		case RIGHT:
			frameMoveValues_.x += running_ ? 1.f : 0.5f; break;
		case UP:
			frameMoveValues_.y += running_ ? 1.f : 0.5f; break;
		case DOWN:
			frameMoveValues_.y -= running_ ? 1.f : 0.5f; break;
		default: 
			break;
	}
}

void FreeCamera::rotate(direction dir, float angle) {
	switch (dir) {
		case LEFT:
			targetRotateValues_.y -= angle; break;
		case RIGHT:
			targetRotateValues_.y += angle; break;
		case UP:
			targetRotateValues_.x -= angle; break;
		case DOWN:
			targetRotateValues_.x += angle; break;
		default: 
			break;
	}
}

void FreeCamera::update(float dt) {
	const float maxMoveSpeed = 3.f * running_ ? 2.f : 1.f; // m/s
	const float linearAcceleration = maxMoveSpeed / 0.5f;	// m/s^2	- we want to reach the target speed in 0.5 seconds
	const float maxRotateSpeed = 4 * PI;	// rad/s
	const float rotationalAcceleration = maxRotateSpeed / 0.25f;	// rad/s^2	- we want to reach the target rotational speed in 0.25 seconds
	
	// compute the speed alteration based on inputs
	float fmv_len = glm::length(frameMoveValues_);
	if (fmv_len > 0.f)
		frameMoveValues_ /= fmv_len;	// normalize direction vector
	frameMoveValues_ *= maxMoveSpeed;	// this vector now represents our target speed in camera space
	// transform it into world space:
	frameMoveValues_ = vec4xyz(getTransform() * glm::vec4(frameMoveValues_, 0.f));
	// how much ground we have to cover to reach that speed
	glm::vec3 delta = frameMoveValues_ - speed_;
	float factor = clamp(linearAcceleration * dt, 0.f, 1.f);
	speed_ += delta * factor;
	position_ += speed_ * dt;
	frameMoveValues_ = glm::vec3(0.f);
	
	// compute rotation alteration based on inputs
	glm::vec2 deltaRot = rotateSpeed_ * dt;	// this is how much we would rotate during this frame
	// we want to rotate by targetRotateValues_ in total from the current state
	//rotateSpeed_.x += rotationalAcceleration * dt * sign
	
}
