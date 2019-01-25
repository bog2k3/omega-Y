#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>

struct TerrainSettings {
	// size settings
	float width = 100.f;	// X axis
	float length = 100.f;	// Z axis
	float minElevation = -5.f;	// minimum Y axis value
	float maxElevation = 15.f;	// maximum Y axis value
	
	// structure settings
	float vertexDensity = 2.f;	// vertices per meter (actual density may be slightly higher due to rounding,
								// but is guaranteed to always be at least the specified value)
	bool irregularEdges = false;	// false keeps the edges as straight as possible, true will remove stretched triangles
									// from the edges, giving an irregular aspect with only 'nice' triangles
	
	// generation parameters
	float relativeRandomJitter = 0.5f;	// random jitter applied to the vertex mesh in the XoZ plane;
										// a value of 1.0 means the amplitude of the jitter is equal 
										// to the initial distance between vertices;
										// this has the effect of producing an irregular (less matrix-like) mesh

	float bigRoughness = 1.f;	// roughness of big features (hills, mountains etc)
	float smallRoughness = 1.f;	// roughness of small features (small local variations in terrain)
};

struct TerrainVertex;
class Viewport;
struct Triangle;
struct RenderData;

class Terrain
{
public:
	Terrain();
	virtual ~Terrain();

	// generate the terrain mesh according to specified settings. This will overwrite the existing data.
	void generate(TerrainSettings const& settings);
	
	// clear all terrain data
	void clear();
	
	void draw(Viewport* vp);
	
private:
	TerrainVertex* pVertices_ = nullptr;
	unsigned nVertices_ = 0;
	std::vector<std::pair<Triangle, bool>> triangles_;	// second indicates if the triangle is in use
	unsigned nUsedTriangles_ = 0;
	TerrainSettings settings_;
	RenderData *renderData_ = nullptr;

	void fixTriangleWinding();
	void computeDisplacements();
	void computeNormals();
	void computeTextureWeights();
	void updateRenderBuffers();
	void cleanupEdges();
	bool isDegenerateTriangle(Triangle const& t) const;
	void loadTextures();
};

#endif // TERRAIN_H
