#include "Terrain.h"

#include "triangulation.h"
#include "HeightMap.h"
#include "PerlinNoise.h"

#include <boglfw/renderOpenGL/Shape3D.h>
#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/math/math3D.h>
#include <boglfw/utils/rand.h>
#include <boglfw/utils/log.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>

#include <new>
#include <algorithm>

struct TerrainVertex {
	static const unsigned nTextures = 4;
	
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv[nTextures];	// uvs for each texture layer
	float texWeight[nTextures];	// weights for each texture layer -> the final color will be a weighted average of each texture
	
	TerrainVertex() = default;
};

struct RenderData {
	unsigned VAO_;
	unsigned VBO_;
	unsigned IBO_;
	unsigned shaderProgram_;
	unsigned iPos_;
	unsigned iNormal_;
	unsigned iColor_;
	unsigned iUV_;
	unsigned iTexWeight_;
	unsigned imPV_;
	unsigned iSampler_;
};

template<>
float nth_elem(TerrainVertex const& v, unsigned n) {
	return	n==0 ? v.pos.x : 
			n==1 ? v.pos.z : 
			0.f;
}

Terrain::Terrain()
{
	renderData_ = new RenderData();
	renderData_->shaderProgram_ = Shaders::createProgram("data/shaders/terrain.vert", "data/shaders/terrain.frag");
	if (!renderData_->shaderProgram_) {
		ERROR("Failed to load terrain shaders!");
		throw std::runtime_error("Failed to load terrain shaders");
	}
	renderData_->iPos_ = glGetAttribLocation(renderData_->shaderProgram_, "pos");
	renderData_->iNormal_ = glGetAttribLocation(renderData_->shaderProgram_, "normal");
	renderData_->iColor_ = glGetAttribLocation(renderData_->shaderProgram_, "color");
	renderData_->iUV_ = glGetAttribLocation(renderData_->shaderProgram_, "uv");
	renderData_->iTexWeight_ = glGetAttribLocation(renderData_->shaderProgram_, "texWeight");
	renderData_->imPV_ = glGetUniformLocation(renderData_->shaderProgram_, "mPV");
	renderData_->iSampler_ = glGetUniformLocation(renderData_->shaderProgram_, "tex");
	glGenVertexArrays(1, &renderData_->VAO_);
	glGenBuffers(1, &renderData_->VBO_);
	glGenBuffers(1, &renderData_->IBO_);
	
	glBindVertexArray(renderData_->VAO_);
	glBindBuffer(GL_ARRAY_BUFFER, renderData_->VBO_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderData_->IBO_);
	glEnableVertexAttribArray(renderData_->iPos_);
	glVertexAttribPointer(renderData_->iPos_, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, pos));
	if (renderData_->iNormal_ > 0) {
		glEnableVertexAttribArray(renderData_->iNormal_);
		glVertexAttribPointer(renderData_->iNormal_, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, normal));
	}
	if (renderData_->iColor_ > 0) {
		glEnableVertexAttribArray(renderData_->iColor_);
		glVertexAttribPointer(renderData_->iColor_, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void*)offsetof(TerrainVertex, color));
	}
	if (renderData_->iUV_ > 0) {
		for (unsigned i=0; i<TerrainVertex::nTextures; i++) {
			glEnableVertexAttribArray(renderData_->iUV_ + i);
			glVertexAttribPointer(renderData_->iUV_ + i, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), 
				(void*)(offsetof(TerrainVertex, uv) + i*sizeof(TerrainVertex::uv[0])));
		}
	}
	if (renderData_->iTexWeight_ > 0) {
		for (unsigned i=0; i<TerrainVertex::nTextures; i++) {
			glEnableVertexAttribArray(renderData_->iTexWeight_ + i);
			glVertexAttribPointer(renderData_->iTexWeight_ + i, 1, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex),
				(void*)(offsetof(TerrainVertex, texWeight) + i*sizeof(TerrainVertex::texWeight[0])));
		}
	}
	glBindVertexArray(0);
}

Terrain::~Terrain()
{
	clear();
	delete renderData_, renderData_ = nullptr;
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
				{0.f, 0.f, 0.f},												// normal
				{1.f, 1.f, 1.f},												// color
				{{0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}},				// uvs
				{0.f, 0.f, 0.f, 0.f}											// tex weights
			};
		}
	
	int trRes = triangulate(pVertices_, nVertices_, triangles_);
	if (trRes < 0) {
		ERROR("Failed to triangulate terrain mesh!");
		return;
	}
	fixTriangleWinding();	// after triangulation some triangles are ccw, we need to fix them

	if (settings_.irregularEdges)
		cleanupEdges();

	computeDisplacements();
	computeNormals();
	
	updateRenderBuffers();
}

void Terrain::fixTriangleWinding() {
	// all triangles must be CW as seen from above
	for (auto &t : triangles_) {
		glm::vec3 n = glm::cross(pVertices_[t.iV2].pos - pVertices_[t.iV1].pos, pVertices_[t.iV3].pos - pVertices_[t.iV1].pos);
		if (n.y < 0) {
			// triangle is CCW, we need to reverse it
			xchg(t.iV1, t.iV3);	// exchange vertices 1 and 3
			xchg(t.iN12, t.iN23); // exchange edges 1-2 and 2-3
		}
	}
}

void Terrain::computeDisplacements() {
	HeightmapParams hparam;
	hparam.width = settings_.width / 2;
	hparam.length = settings_.length / 2;
	hparam.minHeight = settings_.minElevation;
	hparam.maxHeight = settings_.maxElevation;
	HeightMap height(hparam);
	PerlinNoise pnoise(settings_.width, settings_.length);

	glm::vec3 topleft {-settings_.width * 0.5f, 0.f, -settings_.length * 0.5f};
	for (unsigned i=0; i<nVertices_; i++) {
		float u = (pVertices_[i].pos.x - topleft.x) / settings_.width;
		float v = (pVertices_[i].pos.z - topleft.z) / settings_.length;

		float perlinAmp = (settings_.maxElevation - settings_.minElevation) * 0.125f;
		float perlin = pnoise.get(u/16, v/16) * perlinAmp * 0.5
						+ pnoise.get(u/8, v/8) * perlinAmp * 0.3
						+ pnoise.get(u/4, v/4) * perlinAmp * 0.2
						+ pnoise.get(u/2, v/2) * perlinAmp * 0.1
						+ pnoise.get(u/1, v/1) * perlinAmp * 0.05;

		pVertices_[i].pos.y = height.value(u, v) + perlin;

		// debug
		float hr = (pVertices_[i].pos.y - settings_.minElevation) / (settings_.maxElevation - settings_.minElevation);
		pVertices_[i].color = {1.f - hr, hr, 0};
	}
}

void Terrain::computeNormals() {
	for (auto &t : triangles_) {
		glm::vec3 n = glm::cross(pVertices_[t.iV2].pos - pVertices_[t.iV1].pos, pVertices_[t.iV3].pos - pVertices_[t.iV1].pos);
		n = glm::normalize(n);
		pVertices_[t.iV1].normal += n;
		pVertices_[t.iV2].normal += n;
		pVertices_[t.iV3].normal += n;
	}
	for (unsigned i=0; i<nVertices_; i++)
		pVertices_[i].normal = glm::normalize(pVertices_[i].normal);
}

void Terrain::updateRenderBuffers() {
	glBindBuffer(GL_ARRAY_BUFFER, renderData_->VBO_);
	glBufferData(GL_ARRAY_BUFFER, nVertices_ * sizeof(TerrainVertex), pVertices_, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	uint32_t *indices = (uint32_t*)malloc(3 * triangles_.size() * sizeof(uint32_t));
	for (unsigned i=0; i<triangles_.size(); i++) {
		indices[i*3 + 0] = triangles_[i].iV1;
		indices[i*3 + 1] = triangles_[i].iV2;
		indices[i*3 + 2] = triangles_[i].iV3;
	}	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderData_->IBO_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * triangles_.size() * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	free(indices);
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
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glUseProgram(renderData_->shaderProgram_);
	glUniformMatrix4fv(renderData_->imPV_, 1, GL_FALSE, glm::value_ptr(vp->camera()->matProjView()));
	glBindVertexArray(renderData_->VAO_);
	glDrawElements(GL_TRIANGLES, triangles_.size() * 3, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
	glUseProgram(0);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// draw vertex normals
	/*for (unsigned i=0; i<nVertices_; i++) {
		Shape3D::get()->drawLine(pVertices_[i].pos, pVertices_[i].pos+pVertices_[i].normal, {1.f, 0, 1.f});
	}*/
}
