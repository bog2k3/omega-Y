#ifndef CUSTOMMESHRENDERER_H
#define CUSTOMMESHRENDERER_H

#include <glm/fwd.hpp>

class Mesh;
class RenderContext;

class CustomMeshRenderer
{
public:
	CustomMeshRenderer();
	~CustomMeshRenderer();

	void renderMesh(Mesh& mesh, glm::mat4 const& worldTransform, RenderContext const& ctx);

	void setWaterNormalTexture(int texID);

private:
	struct RenderData;

	RenderData* pRenderData_;
};

#endif // CUSTOMMESHRENDERER_H
