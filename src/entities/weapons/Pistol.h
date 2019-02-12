#ifndef __PISTOL_H__
#define __PISTOL_H__

#include "BasicWeapon.h"

class Pistol : public BasicWeapon {
public:
	Pistol();

private:
	static WeaponConfig pistolConfig;
};

#endif // __PISTOL_H__
