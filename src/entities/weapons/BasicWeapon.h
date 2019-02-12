#ifndef __BASIC_WEAPON_H__
#define __BASIC_WEAPON_H__

#include <boglfw/math/transform.h>

class Mesh;
class Viewport;

struct WeaponConfig {
	unsigned magazineSize = 0;
	unsigned reserveSize = 0;
	float reloadTime = 0.f;
};

class BasicWeapon {
public:
	void update(float dt);
	void draw(Transform const& parentTransform);

	// call this to toggle the weapon's primary/secondary trigger on or off
	void toggleTrigger(bool primary, bool on);
	// call this to toggle the weapon's reload action on or off
	// (weapon is reloading while reload is on; reload must be kept on until it's finished)
	void toggleReload(bool on);

	// force an instant reload of up to [count] or magazineSize (whichever is smaller).
	void forceReload(unsigned count);

	// feed some ammo into the weapon's reserve;
	// returns the actual ammount transfered (which can be smaller if there's no more room left).
	unsigned feedAmmo(unsigned count);

	// returns the size of the weapon's magazine (how much ammo can be loaded into it at once).
	virtual unsigned getMagazineSize() const { return config_.magazineSize; }
	// returns loaded ammo into the weapon's magazine
	virtual unsigned getAmmoInMagazine() const { return ammoInMagazine_; }
	// returns ammo that is stored into the reserve, ready to be reloaded into the magazine
	virtual unsigned getAmmoInReserve() const { return ammoInReserve_; }

protected:
	BasicWeapon(WeaponConfig config);

	Mesh* pMesh_ = nullptr;
	Transform localTransform_;
	WeaponConfig config_;
	unsigned ammoInMagazine_ = 0;
	unsigned ammoInReserve_ = 0;
};

#endif // __BASIC_WEAPON_H__
