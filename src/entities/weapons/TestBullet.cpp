#include "TestBullet.h"
#include "../../render/CustomRenderContext.h"
#include "../../render/CustomMeshRenderer.h"

#include <boglfw/renderOpenGL/Mesh.h>

#include <bullet3/BulletCollision/CollisionShapes/btBoxShape.h>

Mesh* TestBullet::pMeshShared_ = nullptr;
std::shared_ptr<btCollisionShape> TestBullet::pCollisionShapeShared_;

TestBullet::TestBullet(glm::vec3 pos, glm::quat orientation, glm::vec3 velocity, glm::quat angularVelocity)
	: BasicProjectile(ProjectileType::TEST_BULLET) {
	if (!pMeshShared_) {
		pMeshShared_ = new Mesh();
		pMeshShared_->createBox(glm::vec3{0.f}, 0.2f, 0.2f, 0.2f);
		// TODO major memory leak here since we never delete this mesh !!!!
	}
	if (!pCollisionShapeShared_) {
		pCollisionShapeShared_ = std::make_shared<btBoxShape>(btVector3(0.1f, 0.1f, 0.1f));
		// TODO major memory leak here since we never delete this collision shape !!!
	}
	PhysBodyConfig bodyCfg;
	bodyCfg.initialAngularVelocity = angularVelocity;
	bodyCfg.initialVelocity = velocity;
	bodyCfg.mass = 0.2f;
	bodyCfg.orientation = orientation;
	bodyCfg.position = pos;
	bodyCfg.shape = pCollisionShapeShared_;
	createPhysicsBody(bodyCfg);
}

TestBullet::~TestBullet() {
}

void TestBullet::draw(RenderContext const& ctx) {
	CustomRenderContext::fromCtx(ctx).meshRenderer->renderMesh(*pMeshShared_, transform_.glMatrix(), ctx);
}
