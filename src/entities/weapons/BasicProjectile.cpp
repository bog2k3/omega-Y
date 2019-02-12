#include "BasicProjectile.h"

BasicProjectile::BasicProjectile(ProjectileType::Types type, glm::vec3 pos, glm::quat orientation, glm::vec3 velocity, glm::quat angularVelocity)
	: projectileType_(type)
	, physBodyProxy_(this)
	, transform_(pos, orientation) {
	physBodyProxy_.onCollision.forward(onCollision);
}

void BasicProjectile::enableCollisionEvent(unsigned otherEntityType, bool enable) {
	physBodyProxy_.collisionCfg[otherEntityType] = enable;
}

void BasicProjectile::update(float dt) {
	// do the physics update here
}
