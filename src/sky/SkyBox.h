#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include <boglfw/entities/Entity.h>
#include <boglfw/math/aabb.h>

#include "../entities/enttypes.h"

#include <string>

class RenderContext;

struct SkyBoxGenerationParams {
};

struct CloudsGenerationParams {
};

// TODO - generate dynamic sky and clouds https://gamedev.stackexchange.com/questions/35724/how-can-i-easily-create-cloud-texture-maps?newreg=ad99d92c77b84ac496492f507a7151c4

class SkyBox : public Entity {
public:
	unsigned getEntityType() const override { return EntityTypes::SKYBOX; }
	FunctionalityFlags getFunctionalityFlags() const override { return FunctionalityFlags::DRAWABLE | FunctionalityFlags::UPDATABLE; }
	AABB getAABB() const override { return AABB::empty(); } // prevent the skybox being retrieved by spatial queries.

	SkyBox();
	~SkyBox();

	// loads a static skybox from a path that contains ./front.png, ./left.png, ./right.png, ./back.png, ./top.png, ./bottom.png
	void load(std::string const& path);

	// generates a dynamic skybox
	void generate(SkyBoxGenerationParams params);

	// generates dynamic clouds
	void generateClouds(CloudsGenerationParams params);

	void clear();

	void draw(RenderContext const& ctx) override;
	void update(float dt) override;

	unsigned getCubeMapTexture() const;

private:
	struct SkyBoxRenderData;
	SkyBoxRenderData* renderData_ = nullptr;

	void setupVAO();
};

#endif // __SKYBOX_H__
