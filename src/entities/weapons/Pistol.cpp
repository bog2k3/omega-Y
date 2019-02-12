#include "Pistol.h"

WeaponConfig Pistol::pistolConfig {
	12,		// magazine size
	60,		// reserve size
	1.f,	// reload time
};

Pistol::Pistol()
	: BasicWeapon(pistolConfig) {
}
