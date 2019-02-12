#ifndef __TEST_BULLET_H__
#define __TEST_BULLET_H__

#include "BasicProjectile.h"
#include "ProjectileTypes.h"

class Mesh;

class TestBullet : public BasicProjectile {
public:
	virtual unsigned getProjectileType() const override { return ProjectileType::TEST_BULLET; }

	// [angularVelocity] is expressed in local coordinates, the rest are in world coordinates
	TestBullet(glm::vec3 pos, glm::quat orientation, glm::vec3 velocity, glm::quat angularVelocity);
	~TestBullet();

	virtual void draw(Viewport*) override;

private:
	static Mesh* pMeshShared_;	// TODO replace static with a MediaManager that loads meshes, textures etc
};

#endif // __TEST_BULLET_H__
