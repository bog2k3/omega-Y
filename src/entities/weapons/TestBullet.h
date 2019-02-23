#ifndef __TEST_BULLET_H__
#define __TEST_BULLET_H__

#include "BasicProjectile.h"
#include "ProjectileTypes.h"

class Mesh;

class TestBullet : public BasicProjectile {
public:
	// [angularVelocity] is expressed in local coordinates, the rest are in world coordinates
	TestBullet(glm::vec3 pos, glm::quat orientation, glm::vec3 velocity, glm::quat angularVelocity);
	~TestBullet();

	virtual void draw(RenderContext const& ctx) override;

private:
	static std::shared_ptr<btCollisionShape> pCollisionShapeShared_;
	static Mesh* pMeshShared_;	// TODO replace static with a MediaManager that loads meshes, textures etc
};

#endif // __TEST_BULLET_H__
