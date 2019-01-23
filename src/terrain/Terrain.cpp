#include "Terrain.h"

#include "triangulation.h"
#include "HeightMap.h"
#include "PerlinNoise.h"

#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/math/math3D.h>
#include <boglfw/utils/rand.h>
#include <boglfw/utils/log.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <new>
#include <algorithm>

struct TerrainVertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 uv[4];	// uvs for each texture layer
	float texWeight[4];	// weights for each texture layer -> the final color will be a weighted average of each texture
	
	TerrainVertex() = default;
};

template<>
float nth_elem(TerrainVertex const& v, unsigned n) {
	return	n==0 ? v.pos.x : 
			n==1 ? v.pos.z : 
			0.f;
}

Terrain::Terrain()
{
}

Terrain::~Terrain()
{
	clear();
}

void Terrain::clear() {
	if (pVertices_)
		free(pVertices_), pVertices_ = nullptr, nVertices_ = 0;
	triangles_.clear();
}

void validateSettings(TerrainSettings const& s) {
	assert(s.width > 0);
	assert(s.length > 0);
	assert(s.maxElevation > s.minElevation);
	assert(s.vertexDensity > 0);
	assert(s.width >= 1.f / s.vertexDensity);
	assert(s.length >= 1.f / s.vertexDensity);
}

void Terrain::generate(TerrainSettings const& settings) {
	validateSettings(settings);
	clear();
	settings_ = settings;
	
	unsigned rows = (unsigned)ceil(settings_.length * settings_.vertexDensity) + 1;
	unsigned cols = (unsigned)ceil(settings_.width * settings_.vertexDensity) + 1;
	float dx = settings_.width / (cols - 1);
	float dz = settings_.length / (rows - 1);
	
	glm::vec3 topleft {-settings_.width * 0.5f, 0.f, -settings.length * 0.5f};
	nVertices_ = rows * cols;
	pVertices_ = (TerrainVertex*)malloc(sizeof(TerrainVertex) * nVertices_);
	
	for (unsigned i=0; i<rows; i++)
		for (unsigned j=0; j<cols; j++) {
			glm::vec2 jitter { randf() * settings_.relativeRandomJitter * dx, randf() * settings_.relativeRandomJitter * dz };
			new(&pVertices_[i*rows + j]) TerrainVertex {
				topleft + glm::vec3(dx * j + jitter.x, 0.f, dz * i + jitter.y),	// position
				{1.f, 1.f, 1.f},												// color
				{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},				// uvs
				{0.f, 0.f, 0.f, 0.f}											// tex weights
			};
		}
	
	int trRes = triangulate(pVertices_, nVertices_, triangles_);
	if (trRes < 0)
		ERROR("Failed to triangulate terrain mesh!");

	if (settings_.irregularEdges)
		cleanupEdges();

	// compute heightmap:
	HeightmapParams hparam;
	hparam.width = settings_.width;
	hparam.length = settings_.length;
	hparam.minHeight = settings_.minElevation;
	hparam.maxHeight = settings_.maxElevation;
	HeightMap height(hparam);
	PerlinNoise pnoise(settings_.width, settings_.length);
	for (unsigned i=0; i<nVertices_; i++) {
		float u = (pVertices_[i].pos.x - topleft.x) / settings_.width;
		float v = (pVertices_[i].pos.z - topleft.z) / settings_.length;
		
		//pVertices_[i].pos.y = height.value(u, v);
		pVertices_[i].pos.y = pnoise.get(u / 8, v / 8) * 8
							+ pnoise.get(u / 4, v / 4) * 4
							+ pnoise.get(u / 2, v / 2) * 2
							+ pnoise.get(u, v);

		// debug
		//pVertices_[i].color = {u, v, 0};
		float hr = (pVertices_[i].pos.y - settings_.minElevation) / (settings_.maxElevation - settings_.minElevation);
		pVertices_[i].color = {1.f - hr, hr, 0};
	}
}

bool Terrain::isDegenerateTriangle(Triangle const& t) const {
	glm::vec3	&a = pVertices_[t.iV1].pos, 
				&b = pVertices_[t.iV2].pos, 
				&c = pVertices_[t.iV3].pos;
	const float minAngle = PI / 8;	// minimum acute angle allowed before the triangle is declared 'degenerate'
	const float minAngleCos = cosf(minAngle);
	float ab_i = 1.f / glm::length(a-b);
	float ac_i = 1.f / glm::length(a-c);
	float bc_i = 1.f / glm::length(b-c);
	float cosAng;
	if (cosAng = glm::dot(b-a, c-a) * ab_i * ac_i, 
		cosAng > minAngleCos)
		// angle A too acute
		return true;
	else if (cosAng = glm::dot(a-b, c-b) * ab_i * bc_i, 
		cosAng > minAngleCos)
		// angle B too acute
		return true;
	else if (cosAng = glm::dot(a-c, b-c) * ac_i * bc_i, 
		cosAng > minAngleCos)
		// angle C too acute
		return true;
	else
		return false;
}

void Terrain::cleanupEdges() {
	// clean up the edges by removing the degenerate triangles that keep the mesh convex.
	// We can do this because we don't care about convexity.
	
	// TODO: this is flawed - must readjust neighbour indexes on all triangles after removing some;
	// also must reiterate until no degenerate edge triangles are found, because there's more than one layer of them
	
	/*triangles_.erase(std::remove_if(triangles_.begin(), triangles_.end(), 
		[this](auto const& t) {
			// check if t is on the edge:
			if (t.iN12 >= 0 && t.iN13 >= 0 && t.iN23 >= 0)	// if it has neighbours on all sides then it's not on the edge
				return false;
			else
				// check if t is 'degenerate':
				return isDegenerateTriangle(t);
		}),
		triangles_.end());
	*/
}

void Terrain::draw(Viewport* vp) {
	for (auto &t : triangles_) {
		Shape3D::get()->drawLine(pVertices_[t.iV1].pos, pVertices_[t.iV2].pos, pVertices_[t.iV1].color);
		Shape3D::get()->drawLine(pVertices_[t.iV1].pos, pVertices_[t.iV3].pos, pVertices_[t.iV3].color);
		Shape3D::get()->drawLine(pVertices_[t.iV2].pos, pVertices_[t.iV3].pos, pVertices_[t.iV2].color);
	}
}
