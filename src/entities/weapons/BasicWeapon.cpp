#include "BasicWeapon.h"

#include <boglfw/math/math3D.h>
#include <boglfw/renderOpenGL/MeshRenderer.h>

BasicWeapon::BasicWeapon(WeaponConfig config)
	: config_(config) {
}

unsigned BasicWeapon::feedAmmo(unsigned count) {
	unsigned amount = min(count, config_.reserveSize - ammoInReserve_);
	ammoInReserve_ += amount;
	return amount;
}

void BasicWeapon::update(float dt) {

}

void BasicWeapon::draw(Transform const& parentTransform) {
	if (pMesh_) {
		Transform tr = parentTransform * localTransform_;
		MeshRenderer::get()->renderMesh(*pMesh_, tr.glMatrix());
	}
}

void BasicWeapon::toggleTrigger(bool primary, bool on) {

}

void BasicWeapon::toggleReload(bool on) {

}

void BasicWeapon::forceReload(unsigned count) {
	ammoInMagazine_ = min(config_.magazineSize, ammoInMagazine_ + count);
}
