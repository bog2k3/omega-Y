#ifndef __BASIC_WEAPON_H__
#define __BASIC_WEAPON_H__

#include <boglfw/math/transform.h>

class Mesh;
class Viewport;

class BasicWeapon {
public:
	void update(float dt);
	void draw(Transform const& parentTransform);

	// call this to toggle the weapon's primary/secondary trigger on or off
	void toggleTrigger(bool primary, bool on);
	// call this to toggle the weapon's reload action on or off
	// (weapon is reloading while reload is on; reload must be kept on until it's finished)
	void toggleReload(bool on);

	// feed some ammo into the weapon's reserve
	void feedAmmo(unsigned count);

	// returns loaded ammo into the weapon's magazine
	virtual unsigned getAmmoInMagazine() const;
	// returns ammo that is stored into the reserve, ready to be reloaded into the magazine
	virtual unsigned getAmmoInReserve() const;

protected:
	BasicWeapon() = default;

	Mesh* pMesh_ = nullptr;
	Transform localTransform_;
};

#endif // __BASIC_WEAPON_H__
