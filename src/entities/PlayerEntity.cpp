#include "PlayerEntity.h"
#include <boglfw/math/math3D.h>
#include <boglfw/math/aabb.h>

PlayerEntity::PlayerEntity(glm::vec3 position, glm::vec3 direction)
{
}

PlayerEntity::~PlayerEntity()
{
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
	/*if (transformDirty_) {
		glm::vec3 up {0.f, 1.f, 0.f};
		transform_ = buildMatrixFromOrientation(position_, direction_, up);
		transformDirty_ = false;
	}
	return transform_;*/
	return glm::mat4(1.f);
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