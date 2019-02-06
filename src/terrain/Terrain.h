#ifndef TERRAIN_H
#define TERRAIN_H

#include "../physics/PhysBodyMeta.h"
#include "../entities/enttypes.h"
#include "../BSP/BSP.h"

#include <boglfw/entities/Entity.h>

#include <bullet3/LinearMath/btScalar.h>

#include <vector>

struct TerrainSettings {
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

	float bigRoughness = 1.f;	// roughness of big features (hills, mountains etc)
	float smallRoughness = 1.f;	// roughness of small features (small local variations in terrain)
};

class Viewport;
struct Triangle;
class Water;
class btHeightfieldTerrainShape;
class btRigidBody;

class Terrain : public Entity {
public:
	Terrain();
	virtual ~Terrain();

	FunctionalityFlags getFunctionalityFlags() const override { return FunctionalityFlags::DRAWABLE; }
	unsigned getEntityType() const override { return EntityTypes::TERRAIN; }

	// generate the terrain mesh according to specified settings. This will overwrite the existing data.
	void generate(TerrainSettings const& settings);

	// clear all terrain data
	void clear();

	void draw(Viewport* vp);
	void setWireframeMode(bool wireframe) { renderWireframe_ = wireframe; }

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
	TerrainSettings settings_;
	RenderData *renderData_ = nullptr;
	bool renderWireframe_ = false;
	Water* pWater_ = nullptr;
	BSPTree<unsigned, false> *pBSP_ = nullptr;

	PhysBodyMeta physicsBodyMeta_;
	btHeightfieldTerrainShape* physicsShape_ = nullptr;
	btScalar *heightFieldValues_ = nullptr;

	void loadTextures();
	void fixTriangleWinding();
	void computeDisplacements();
	void computeNormals();
	void computeTextureWeights();
	void updateRenderBuffers();
	void updatePhysics();
};

#endif // TERRAIN_H
