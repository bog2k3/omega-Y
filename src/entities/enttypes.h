#ifndef __ENTTYPES_H__
#define __ENTTYPES_H__

namespace EntityTypes {
	enum GameEntityTypes : unsigned {
		FREE_CAMERA				= 101,	// we start at 101 to avoid collision with EntityTypes::EType from boglfw
		PLAYER					= 102,
		TERRAIN					= 103,
	};
} // namespace

#endif // __ENTTYPES_H__
