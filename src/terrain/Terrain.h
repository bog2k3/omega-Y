#ifndef TERRAIN_H
#define TERRAIN_H

#include "../physics/PhysBodyProxy.h"
#include "../entities/enttypes.h"
#include "TerrainConfig.h"
#include "../render/programs/ShaderTerrain.h"
#include "../progress.h"

#define ENABLE_BSP_DEBUG_DRAW
#include "../BSP/BSP.h"

#include <boglfw/entities/Entity.h>

#include <bullet3/LinearMath/btScalar.h>

#include <vector>

class Viewport;
struct Triangle;
class Water;
class TriangleAABBGenerator;
class btHeightfieldTerrainShape;
class btRigidBody;
struct TerrainVertex;

class Terrain : public Entity {
public:
	static Progress loadTextures(unsigned step);
	static void unloadAllResources();

	// specify previewMode=true to enable "preview" (simplified) rendering for rendering in the menu.
	explicit Terrain(bool previewMode=false);
	virtual ~Terrain();

	FunctionalityFlags getFunctionalityFlags() const override { return FunctionalityFlags::DRAWABLE; }
	unsigned getEntityType() const override { return EntityTypes::TERRAIN; }

	// generate the terrain mesh according to specified config. This will overwrite the existing data.
	// Render buffers and Physics data structures will not be generated at this point, thus allowing the user to make modifications
	// to the terrain geometry before that.
	// Call finishGenerate() to generate these objects after you're done.
	void generate(TerrainConfig const& config);

	// Generate the render buffers and physics data structures
	void finishGenerate();

	// clear all terrain data
	void clear();

	void draw(RenderContext const& ctx) override;
	void update(float dt) override;
	void setWireframeMode(bool wireframe, bool thickLines=false) { renderWireframe_ = wireframe; thickWireframeLines_ = thickLines; }

	float getHeightValue(glm::vec3 const& where) const; // only x and z coords are used from the input point
	TerrainConfig const& getConfig() const { return config_; }
	const float* getHeightField() const { return heightFieldValues_; }
	glm::ivec2 getGridSize() const { return {cols_, rows_}; }

	// set a 2D texture for water reflection
	void setWaterReflectionTex(unsigned texId);
	// set a 2D texture to be used as water refraction
	void setWaterRefractionTex(unsigned texId_2D, unsigned texId_Cube);

	int getWaterNormalTexture() const;

private:
	struct RenderData;
	friend class TriangleAABBGenerator;

	unsigned rows_ = 0;
	unsigned cols_ = 0;
	std::pair<float, float> gridSpacing_;
	TerrainVertex* pVertices_ = nullptr;
	unsigned nVertices_ = 0;
	std::vector<Triangle> triangles_;
	TerrainConfig config_;
	RenderData *renderData_ = nullptr;
	bool renderWireframe_ = false;
	bool thickWireframeLines_ = true;
	bool previewMode_ = false;
	Water* pWater_ = nullptr;
	TriangleAABBGenerator* triangleAABBGenerator_ = nullptr;
	BSPTree<unsigned> *pBSP_ = nullptr;

	PhysBodyProxy physicsBodyMeta_;
	float *heightFieldValues_ = nullptr;

	void setupVAO();
	void fixTriangleWinding();
	void computeDisplacements(uint32_t seed);
	void meltEdges(unsigned xRadius, unsigned zRadius);
	void computeNormals();
	void computeTextureWeights();
	void updateRenderBuffers();
	void updatePhysics();
};

#endif // TERRAIN_H
