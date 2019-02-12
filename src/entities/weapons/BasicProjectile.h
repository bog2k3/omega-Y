#ifndef __BASIC_PROJECTILE_H__
#define __BASIC_PROJECTILE_H__

#include <boglfw/entities/Entity.h>
#include "ProjectileTypes.h"
#include "../enttypes.h"
#include "../../physics/PhysBodyProxy.h"

class BasicProjectile : public Entity {
public:
	virtual FunctionalityFlags getFunctionalityFlags() const override { return FunctionalityFlags::UPDATABLE | FunctionalityFlags::DRAWABLE; };
	virtual unsigned getEntityType() const override { return EntityTypes::PROJECTILE; }

	ProjectileType::Types getProjectileType() const { return projectileType_; }

	virtual ~BasicProjectile() {}

	virtual void update(float dt) override;

	// onCollision event
	Event<void(CollisionEvent const&)> onCollision;
	// use this to enable/disable collision events against other entity types.
	void enableCollisionEvent(unsigned otherEntityType, bool enable);

protected:
	BasicProjectile(ProjectileType::Types type);
	void createPhysicsBody(PhysBodyConfig const& cfg);

private:
	PhysBodyProxy physBodyProxy_;
	ProjectileType::Types projectileType_;
};

#endif // __BASIC_PROJECTILE_H__
