#ifndef WATER_H
#define WATER_H

#include "../progress.h"

#include <vector>
#include <memory>

struct WaterParams {
	float innerRadius = 50.f;	// radius of 'detailed' water mesh -> should cover the playable area
	float outerExtent = 100.f;	// extend from the innerRadius to make the water appear infinite - this area will have fewer vertices
	float vertexDensity = 0.1f;	// vertices per meter
	bool constrainToCircle = false;	// true to enable detailed vertex generation only within the circle of radius 'innerRadius'
									// if false, a square of length 2*innerRadius will be used instead (faster)
};

class RenderContext;
struct Triangle;
struct WaterVertex;

class Water {
public:
	static Progress loadTextures(unsigned step);
	static void unloadAllResources();

	Water();
	virtual ~Water();

	void generate(WaterParams params);

	void setReflectionTexture(unsigned texId_2D);
	void setRefractionTexture(unsigned texId, unsigned texId_Cube);

	void update(float dt);
	void draw(RenderContext const& ctx);

	int getNormalTexture() const;

private:
	struct RenderData;

	WaterParams params_;
	RenderData *renderData_ = nullptr;
	WaterVertex* pVertices_ = nullptr;
	unsigned nVertices_ = 0;
	std::vector<Triangle> triangles_;

	void validateParams(WaterParams const& p);
	void clear();
	void fixTriangleWinding();
	void updateRenderBuffers();
};

#endif // WATER_H
