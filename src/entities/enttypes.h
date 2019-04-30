#ifndef __ENTTYPES_H__
#define __ENTTYPES_H__

namespace EntityTypes {
	enum GameEntityTypes : unsigned {
		FREE_CAMERA				= 101,	// we start at 101 to avoid collision with EntityTypes::EType from boglfw
		PLAYER					= 102,
		TERRAIN					= 103,
		SKYBOX					= 104,
		PROJECTILE				= 105,	// all projectiles have this entity type
	};
} // namespace

#endif // __ENTTYPES_H__
