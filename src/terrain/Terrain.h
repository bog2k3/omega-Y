#ifndef TERRAIN_H
#define TERRAIN_H

#include "../physics/PhysBodyProxy.h"
#include "../entities/enttypes.h"

#define ENABLE_BSP_DEBUG_DRAW
#include "../BSP/BSP.h"

#include <boglfw/entities/Entity.h>

#include <bullet3/LinearMath/btScalar.h>

#include <vector>

struct TerrainConfig {
	// size settings
	float width = 100.f;	// X axis
	float length = 100.f;	// Z axis
	float minElevation = -5.f;	// minimum Y axis value
	float maxElevation = 15.f;	// maximum Y axis value
	float seaLevel = 0.f;

	// structure settings
	float vertexDensity = 2.f;	// vertices per meter (actual density may be slightly higher due to rounding,
								// but is guaranteed to always be at least the specified value)

	// generation parameters
	float relativeRandomJitter = 0.5f;	// random jitter applied to the vertex mesh in the XoZ plane;
										// a value of 1.0 means the amplitude of the jitter is equal
										// to the initial distance between vertices;
										// this has the effect of producing an irregular (less matrix-like) mesh

	float bigRoughness = 0.5f;		// between [0.0 and 1.0] -> controls roughness of big features (hills/cliffs etc)
	float smallRoughness = 1.f;		// roughness of small features (small local variations in terrain)
};

class Viewport;
struct Triangle;
class Water;
class TriangleAABBGenerator;
class btHeightfieldTerrainShape;
class btRigidBody;

class Terrain : public Entity {
public:
	Terrain();
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

	void draw(Viewport* vp) override;
	void update(float dt) override;
	void setWireframeMode(bool wireframe) { renderWireframe_ = wireframe; }

	float getHeightValue(glm::vec3 const& where) const; // only x and z coords are used from the input point
	TerrainConfig const& getConfig() const { return config_; }
	const float* getHeightField() const { return heightFieldValues_; }
	glm::ivec2 getGridSize() const { return {cols_, rows_}; }

	// set a cube map texture for water reflection
	void setWaterReflectionTex(unsigned texId);

	struct TerrainVertex;

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
	Water* pWater_ = nullptr;
	TriangleAABBGenerator* triangleAABBGenerator_ = nullptr;
	BSPTree<unsigned> *pBSP_ = nullptr;

	PhysBodyProxy physicsBodyMeta_;
	float *heightFieldValues_ = nullptr;

	void loadTextures();
	void fixTriangleWinding();
	void computeDisplacements();
	void meltEdges(unsigned xRadius, unsigned zRadius);
	void computeNormals();
	void computeTextureWeights();
	void updateRenderBuffers();
	void updatePhysics();
};

#endif // TERRAIN_H
