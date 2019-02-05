#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include <string>

class Viewport;

struct SkyBoxGenerationParams {
};

struct CloudsGenerationParams {
};

class SkyBox {
public:
	SkyBox();
	~SkyBox();

	// loads a static skybox from a path that contains ./front.png, ./left.png, ./right.png, ./back.png, ./top.png, ./bottom.png
	void load(std::string const& path);

	// generates a dynamic skybox
	void generate(SkyBoxGenerationParams params);

	// generates dynamic clouds
	void generateClouds(CloudsGenerationParams params);

	void clear();

	void draw(Viewport* vp);
	void update(float dt);

private:
	struct SkyBoxRenderData;
	SkyBoxRenderData* renderData_ = nullptr;
};

#endif // __SKYBOX_H__
