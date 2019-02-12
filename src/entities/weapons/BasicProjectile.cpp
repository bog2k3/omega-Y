#include "BasicProjectile.h"

BasicProjectile::BasicProjectile(ProjectileType::Types type)
	: projectileType_(type)
	, physBodyProxy_(this) {
	physBodyProxy_.onCollision.forward(onCollision);
}

void BasicProjectile::createPhysicsBody(PhysBodyConfig const& cfg) {
	physBodyProxy_.createBody(cfg);
}

void BasicProjectile::enableCollisionEvent(unsigned otherEntityType, bool enable) {
	physBodyProxy_.collisionCfg[otherEntityType] = enable;
}

void BasicProjectile::update(float dt) {
	physBodyProxy_.updateTransform(transform_);
}
