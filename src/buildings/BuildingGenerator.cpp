#include "BuildingGenerator.h"
#include "../terrain/Terrain.h"

#include <boglfw/utils/rand.h>
#include <boglfw/entities/Box.h>
#include <boglfw/World.h>

// blurs a height field over a given radius
void blurHeightField(const float* in, int rows, int cols, float radius, float* out) {
	int irad = round(radius);
	float rsq = sqr(radius);
	for (int i=0; i<rows; i++) {
		for (int j=0; j<cols; j++) {
			float val = 0;
			float denom = 0;
			for (int ii=-irad; ii<=irad; ii++) {
				for (int jj=-irad; jj<=irad; jj++) {
					float cubicFactor = (rsq - sqr(ii)+sqr(jj)) / rsq;
					if (cubicFactor <= 0)
						continue;	// the point is outside blur radius
					int ri = i + ii;	// row index
					int ci = j + jj;	// column index
					if (ri < 0 || ri >= rows || ci < 0 || ci >= cols)
						continue; // we're outside the field
					val += in[ri*rows + ci] * cubicFactor;
					denom += cubicFactor;
				}
			}
			out[i*rows + j] = val / denom;
		}
	}
}

// computes a matrix of gradient vectors for each entry in the heightfield
void computeFieldGradient(const float* in, int rows, int cols, glm::vec2* out) {

}

void BuildingGenerator::generate(BuildingsSettings const& settings, Terrain &terrain) {
	glm::ivec2 gridSize = terrain.getGridSize();
	float samplePointDensity = 0.0045f; // per meter squared -> this will yield a distance of about 15 meters between sample points
	// prepare heightfield for finding locations:
	float* fHeights = (float*)malloc(sizeof(float) * gridSize.x * gridSize.y);
	float blurRadius = 1.f / sqrt(samplePointDensity) * 0.25f;	// one quarter of the distance between sample points
	blurHeightField(terrain.getHeightField(), gridSize.y, gridSize.x, blurRadius, fHeights);
	// find suitable locations for castles
	unsigned nSamplePoints = terrain.getConfig().width * terrain.getConfig().length * samplePointDensity;
	for (unsigned i=0; i<nSamplePoints; i++) {
		glm::ivec2 sp { randi(gridSize.x - 1), randi(gridSize.y - 1) };
		glm::vec3 wp { (sp.x / (float)gridSize.x - 0.5f) * terrain.getConfig().width, 0.f,
						(sp.y / (float)gridSize.y - 0.5f) * terrain.getConfig().length };
		wp.y = fHeights[sp.y * gridSize.y + sp.x];

		std::shared_ptr<Box> spB = std::make_shared<Box>(0.2f, 1.f, 0.2f);
		spB->getTransform().setPosition(wp);
		World::getInstance().takeOwnershipOf(spB);
	}
	// we now have our sample points, we need to move them around according to local gradients to find local maximums
}
