#ifndef __BASIC_PROJECTILE_H__
#define __BASIC_PROJECTILE_H__

#include <boglfw/entities/Entity.h>
#include "enttypes.h"

class BasicProjectile : public Entity {
public:
	virtual FunctionalityFlags getFunctionalityFlags() const override { return FunctionalityFlags::UPDATABLE | FunctionalityFlags::DRAWABLE; };
	virtual unsigned getEntityType() const override { return EntityTypes::PROJECTILE; }

	virtual unsigned getProjectileType() const = 0;

	virtual void update(float dt) override;

private:
};

#endif // __BASIC_PROJECTILE_H__
