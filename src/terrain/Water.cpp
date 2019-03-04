#include "Water.h"

#include "triangulation.h"
#include "PerlinNoise.h"
#include "../CustomRenderContext.h"

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
	unsigned iFog_;
	unsigned iUV_;
	unsigned imPV_;
	unsigned iEyePos_;
	unsigned iTime_;
	unsigned iTexture1_;
	unsigned iTexReflection_;
	unsigned iTexRefraction_;

	unsigned textureNormal_;
	unsigned textureReflection_;
	unsigned textureRefraction_;

	float time_ = 0.f;
};

struct Water::WaterVertex {
	glm::vec3 pos;
	glm::vec2 uv;
	float fog;

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
	glGenVertexArrays(1, &renderData_->VAO_);
	glGenBuffers(1, &renderData_->VBO_);
	glGenBuffers(1, &renderData_->IBO_);

	Shaders::createProgram("data/shaders/water.vert", "data/shaders/water.frag", [this](unsigned id) {
		renderData_->shaderProgram_ = id;
		if (!renderData_->shaderProgram_) {
			ERROR("Failed to load water shaders!");
			return;
		}
		renderData_->iPos_ = glGetAttribLocation(renderData_->shaderProgram_, "pos");
		renderData_->iFog_ = glGetAttribLocation(renderData_->shaderProgram_, "fog");
		renderData_->iUV_ = glGetAttribLocation(renderData_->shaderProgram_, "uv");
		renderData_->iEyePos_ = glGetUniformLocation(renderData_->shaderProgram_, "eyePos");
		renderData_->iTime_ = glGetUniformLocation(renderData_->shaderProgram_, "time");
		renderData_->imPV_ = glGetUniformLocation(renderData_->shaderProgram_, "mPV");
		renderData_->iTexture1_ = glGetUniformLocation(renderData_->shaderProgram_, "textureNormal");
		renderData_->iTexReflection_ = glGetUniformLocation(renderData_->shaderProgram_, "textureReflection");
		renderData_->iTexRefraction_ = glGetUniformLocation(renderData_->shaderProgram_, "textureRefraction");

		glBindVertexArray(renderData_->VAO_);
		glBindBuffer(GL_ARRAY_BUFFER, renderData_->VBO_);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderData_->IBO_);
		glEnableVertexAttribArray(renderData_->iPos_);
		glVertexAttribPointer(renderData_->iPos_, 3, GL_FLOAT, GL_FALSE, sizeof(WaterVertex),
			(void*)offsetof(WaterVertex, pos));
		if (renderData_->iFog_ > 0) {
			glEnableVertexAttribArray(renderData_->iFog_);
			glVertexAttribPointer(renderData_->iFog_, 1, GL_FLOAT, GL_FALSE, sizeof(WaterVertex),
				(void*)offsetof(WaterVertex, fog));
		}
		if (renderData_->iUV_ > 0) {
			glEnableVertexAttribArray(renderData_->iUV_);
			glVertexAttribPointer(renderData_->iUV_, 2, GL_FLOAT, GL_FALSE, sizeof(WaterVertex),
				(void*)(offsetof(WaterVertex, uv)));
		}
		glBindVertexArray(0);
	});

	loadTextures();
}

Water::~Water()
{
	if (renderData_)
		delete renderData_, renderData_ = nullptr;
}

void Water::setReflectionTexture(unsigned reflectionTexCubeMapId) {
	renderData_->textureReflection_ = reflectionTexCubeMapId;
}

void Water::setRefractionTexture(unsigned refractionTexId) {
	renderData_->textureRefraction_ = refractionTexId;
}

void Water::loadTextures() {
	renderData_->textureNormal_ = TextureLoader::loadFromPNG("data/textures/water/normal2.png", false);
	glBindTexture(GL_TEXTURE_2D, renderData_->textureNormal_);
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
	float wTexW = 100.f;	// world width of water texture
	float wTexH = 100.f;	// world height of water texture
	// compute water vertices
	for (unsigned i=0; i<rows; i++)
		for (unsigned j=0; j<cols; j++) {
			new(&pVertices_[i*rows + j]) WaterVertex {
				topleft + glm::vec3(dx * j, 0, dz * i),	// position
				{dx*j / wTexW, dz*i / wTexH},			// uv
				0.f										// fog
			};
		}
	// compute skirt vertices
	for (unsigned i=0; i<nSkirtVerts; i++) {
		float x = extentRadius * cosf(i*skirtVertSector);
		float z = extentRadius * sinf(i*skirtVertSector);
		new(&pVertices_[rows*cols+i]) WaterVertex {
			{ x, 0, z },														// position
			{(x+params_.innerRadius) / wTexW, (z+params_.innerRadius) / wTexH},	// uv
			1.f																	// fog
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

void Water::draw(RenderContext const& ctx) {
	if (!renderData_->shaderProgram_) {
		return;
	}
	// configure backface culling
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	// set-up shader, vertex buffer and uniforms
	glUseProgram(renderData_->shaderProgram_);
	glUniform3fv(renderData_->iEyePos_, 1, &ctx.viewport.camera().position().x);
	glUniform1f(renderData_->iTime_, renderData_->time_);
	// set-up textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderData_->textureNormal_);
	glUniform1i(renderData_->iTexture1_, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderData_->textureReflection_);
	glUniform1i(renderData_->iTexReflection_, 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, renderData_->textureRefraction_);
	glUniform1i(renderData_->iTexRefraction_, 2);
	glUniformMatrix4fv(renderData_->imPV_, 1, GL_FALSE, glm::value_ptr(ctx.viewport.camera().matProjView()));
	glBindVertexArray(renderData_->VAO_);
	// do the drawing
	glDrawElements(GL_TRIANGLES, triangles_.size() * 3, GL_UNSIGNED_INT, nullptr);
	// unbind stuff
	glBindVertexArray(0);
	glUseProgram(0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void Water::update(float dt) {
	renderData_->time_ += dt;
}
