#include "Water.h"

#include "triangulation.h"
#include "PerlinNoise.h"

#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/renderOpenGL/TextureLoader.h>
#include <boglfw/math/math3D.h>
#include <boglfw/utils/rand.h>
#include <boglfw/utils/log.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>

struct Water::RenderData {
	unsigned VAO_;
	unsigned VBO_;
	unsigned IBO_;
	unsigned shaderProgram_;
	unsigned iPos_;
	unsigned iColor_;
	unsigned iUV_;
	unsigned imPV_;
	unsigned iSampler_;
	
	unsigned textureId_;
};

struct Water::WaterVertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 uv;
	
	WaterVertex() = default;
};

template<>
float nth_elem(Water::WaterVertex const& v, unsigned n) {
	return	n==0 ? v.pos.x : 
			n==1 ? v.pos.z : 
			0.f;
}

Water::Water()
{
	renderData_ = new RenderData;
	renderData_->shaderProgram_ = Shaders::createProgram("data/shaders/water.vert", "data/shaders/water.frag");
	if (!renderData_->shaderProgram_) {
		ERROR("Failed to load water shaders!");
		throw std::runtime_error("Failed to load water shaders");
	}
	renderData_->iPos_ = glGetAttribLocation(renderData_->shaderProgram_, "pos");
	renderData_->iColor_ = glGetAttribLocation(renderData_->shaderProgram_, "color");
	renderData_->iUV_ = glGetAttribLocation(renderData_->shaderProgram_, "uv");
	renderData_->imPV_ = glGetUniformLocation(renderData_->shaderProgram_, "mPV");
	renderData_->iSampler_ = glGetUniformLocation(renderData_->shaderProgram_, "tex");
	glGenVertexArrays(1, &renderData_->VAO_);
	glGenBuffers(1, &renderData_->VBO_);
	glGenBuffers(1, &renderData_->IBO_);
	
	glBindVertexArray(renderData_->VAO_);
	glBindBuffer(GL_ARRAY_BUFFER, renderData_->VBO_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderData_->IBO_);
	glEnableVertexAttribArray(renderData_->iPos_);
	glVertexAttribPointer(renderData_->iPos_, 3, GL_FLOAT, GL_FALSE, sizeof(WaterVertex),
		(void*)offsetof(WaterVertex, pos));
	if (renderData_->iColor_ > 0) {
		glEnableVertexAttribArray(renderData_->iColor_);
		glVertexAttribPointer(renderData_->iColor_, 3, GL_FLOAT, GL_FALSE, sizeof(WaterVertex),
			(void*)offsetof(WaterVertex, color));
	}
	if (renderData_->iUV_ > 0) {
		glEnableVertexAttribArray(renderData_->iUV_);
		glVertexAttribPointer(renderData_->iUV_, 2, GL_FLOAT, GL_FALSE, sizeof(WaterVertex), 
			(void*)(offsetof(WaterVertex, uv)));
	}
	glBindVertexArray(0);

	loadTextures();
}

Water::~Water()
{
	if (renderData_)
		delete renderData_, renderData_ = nullptr;
}

void Water::loadTextures() {
	renderData_->textureId_ = TextureLoader::loadFromPNG("data/textures/water/water1.png", nullptr, nullptr);
	glBindTexture(GL_TEXTURE_2D, renderData_->textureId_);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Water::validateParams(WaterParams const& p) {
	assert(p.innerRadius > 0);
	assert(p.outerExtent > 0);
	assert(p.innerRadius > 1.f / p.vertexDensity);
}

void Water::clear() {
	if (pVertices_)
		free(pVertices_), pVertices_ = nullptr, nVertices_ = 0;
	triangles_.clear();
}

void Water::generate(WaterParams params) {
	validateParams(params);
	clear();
	params_ = params;
	
	// TODO implement constrainToCircle
	
	unsigned rows = (unsigned)ceil(2 * params_.innerRadius * params_.vertexDensity) + 1;
	unsigned cols = (unsigned)ceil(2 * params_.innerRadius * params_.vertexDensity) + 1;
	
	// generate 'skirt' vertices that will surround the main water body to the outerExtent
	float extentRadius = params_.innerRadius + params_.outerExtent;
	float skirtVertSpacing = 30.f; // meters
	unsigned nSkirtVerts = (2 * PI * extentRadius) / skirtVertSpacing;
	float skirtVertSector = 2 * PI / nSkirtVerts; // sector size between two skirt vertices
	nVertices_ = rows * cols + nSkirtVerts;
	pVertices_ = (WaterVertex*)malloc(sizeof(WaterVertex) * nVertices_);
	
	glm::vec3 topleft {-params_.innerRadius, 0.f, -params_.innerRadius};
	float dx = params_.innerRadius * 2.f / (cols - 1);
	float dz = params_.innerRadius * 2.f / (rows - 1);
	float wTexW = 5.f;	// world width of water texture
	float wTexH = 5.f;	// world height of water texture
	// compute water vertices
	for (unsigned i=0; i<rows; i++)
		for (unsigned j=0; j<cols; j++) {
			new(&pVertices_[i*rows + j]) WaterVertex {
				topleft + glm::vec3(dx * j, params_.waterLevel, dz * i),	// position
				{1.f, 1.f, 1.f},											// color
				{dx*j / wTexW, dz*i / wTexH}								// uv
			};
		}
	// compute skirt vertices
	for (unsigned i=0; i<nSkirtVerts; i++) {
		float x = extentRadius * cosf(i*skirtVertSector);
		float z = extentRadius * sinf(i*skirtVertSector);
		new(&pVertices_[rows*cols+i]) WaterVertex {
			{ x, params_.waterLevel, z },										// position
			{ 1.f, 1.f, 1.f },													// color
			{(x+params_.innerRadius) / wTexW, (z+params_.innerRadius) / wTexH}	// uv
		};
	}
	
	int trRes = triangulate(pVertices_, nVertices_, triangles_);
	if (trRes < 0) {
		ERROR("Failed to triangulate water mesh!");
		return;
	}
	fixTriangleWinding();	// after triangulation some triangles are ccw, we need to fix them
	
	updateRenderBuffers();
}

void Water::fixTriangleWinding() {
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

void Water::updateRenderBuffers() {
	glBindBuffer(GL_ARRAY_BUFFER, renderData_->VBO_);
	glBufferData(GL_ARRAY_BUFFER, nVertices_ * sizeof(WaterVertex), pVertices_, GL_STATIC_DRAW);
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

void Water::draw(Viewport* vp) {
	// set-up textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderData_->textureId_);
	// configure backface culling
	glDisable(GL_CULL_FACE);
	// set-up shader, vertex buffer and uniforms
	glUseProgram(renderData_->shaderProgram_);
	glUniformMatrix4fv(renderData_->imPV_, 1, GL_FALSE, glm::value_ptr(vp->camera()->matProjView()));
	glUniform1i(renderData_->iSampler_, 0);
	glBindVertexArray(renderData_->VAO_);
	// do the drawing
	glDrawElements(GL_TRIANGLES, triangles_.size() * 3, GL_UNSIGNED_INT, nullptr);
	// unbind stuff
	glBindVertexArray(0);
	glUseProgram(0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}