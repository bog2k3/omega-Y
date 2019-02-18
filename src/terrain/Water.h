#ifndef WATER_H
#define WATER_H

#include <vector>

struct WaterParams {
	float waterLevel = 0.f;
	float innerRadius = 50.f;	// radius of 'detailed' water mesh -> should cover the playable area
	float outerExtent = 100.f;	// extend from the innerRadius to make the water appear infinite - this area will have fewer vertices
	float vertexDensity = 0.1f;	// vertices per meter
	bool constrainToCircle = false;	// true to enable detailed vertex generation only within the circle of radius 'innerRadius'
									// if false, a square of length 2*innerRadius will be used instead (faster)
};

class Viewport;
struct RenderData;
struct Triangle;

class Water {
public:
	Water();
	virtual ~Water();

	void generate(WaterParams params);

	void setReflectionTexture(unsigned reflectionTexCubeMapId);

	void update(float dt);
	void draw(Viewport* v);

	struct WaterVertex;

private:
	struct RenderData;

	WaterParams params_;
	RenderData *renderData_ = nullptr;
	WaterVertex* pVertices_ = nullptr;
	unsigned nVertices_ = 0;
	std::vector<Triangle> triangles_;

	void loadTextures();
	void validateParams(WaterParams const& p);
	void clear();
	void fixTriangleWinding();
	void updateRenderBuffers();
};

#endif // WATER_H
