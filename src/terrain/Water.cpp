#include "Water.h"

#include "triangulation.h"
#include "PerlinNoise.h"
#include "../render/CustomRenderContext.h"

#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/renderOpenGL/ShaderProgram.h>
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

	static ShaderProgram shaderProgram_;
	// int iAspectRatio_;
	static int iTexNormal;
	static int iTexFoam;
	static int iTexReflection_2D_;
	static int iTexRefraction_Cube_;
	static int iTexRefraction_;

	static unsigned textureNormal_;
	static unsigned textureFoam;

	unsigned textureReflection_2D_;
	unsigned textureRefraction_Cube_;
	unsigned textureRefraction_;
};

ShaderProgram Water::RenderData::shaderProgram_;
int Water::RenderData::iTexNormal = -1;
int Water::RenderData::iTexReflection_2D_ = -1;
int Water::RenderData::iTexRefraction_Cube_ = -1;
int Water::RenderData::iTexRefraction_ = -1;
int Water::RenderData::iTexFoam = -1;
unsigned Water::RenderData::textureNormal_ = 0;
unsigned Water::RenderData::textureFoam = 0;

struct Water::WaterVertex {
	glm::vec3 pos;
	float fog;
};

Progress Water::loadShaders(unsigned step) {
	RenderData::shaderProgram_.defineVertexAttrib("pos", GL_FLOAT, 3, sizeof(WaterVertex), offsetof(WaterVertex, pos));
	RenderData::shaderProgram_.defineVertexAttrib("fog", GL_FLOAT, 1, sizeof(WaterVertex), offsetof(WaterVertex, fog));

	RenderData::shaderProgram_.onProgramReloaded.add([](auto const& prog) {
		RenderData::iTexNormal = RenderData::shaderProgram_.getUniformLocation("textureWaterNormal");
		RenderData::iTexFoam = RenderData::shaderProgram_.getUniformLocation("textureFoam");
		RenderData::iTexReflection_2D_ = RenderData::shaderProgram_.getUniformLocation("textureReflection2D");
		RenderData::iTexRefraction_ = RenderData::shaderProgram_.getUniformLocation("textureRefraction");
		RenderData::iTexRefraction_Cube_ = RenderData::shaderProgram_.getUniformLocation("textureRefractionCube");
	});
	if (!RenderData::shaderProgram_.load("data/shaders/water.vert", "data/shaders/water.frag")) {
		ERROR("Failed to load water shaders!");
	}

	return {1, 1};
}

Progress Water::loadTextures(unsigned step) {
	switch (step) {
		case 0:
			RenderData::textureNormal_ = TextureLoader::loadFromPNG("data/textures/water/normal.png", false);
			glBindTexture(GL_TEXTURE_2D, RenderData::textureNormal_);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);//GL_NEAREST_MIPMAP_LINEAR);// GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
		case 1:
			RenderData::textureFoam = TextureLoader::loadFromPNG("data/textures/water/foam.png", true);
			glBindTexture(GL_TEXTURE_2D, RenderData::textureFoam);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		break;
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	return {step+1, 2};
}

void Water::unloadAllResources() {
	RenderData::shaderProgram_.reset();
	glDeleteTextures(1, &RenderData::textureNormal_), RenderData::textureNormal_ = 0;
	glDeleteTextures(1, &RenderData::textureFoam), RenderData::textureFoam = 0;
}

template<>
float nth_elem(Water::WaterVertex const& v, unsigned n) {
	return	n==0 ? v.pos.x :
			n==1 ? v.pos.z :
			0.f;
}

Water::Water(std::shared_ptr<UniformPack> unifCommon)
{
	renderData_ = new RenderData;
	glGenVertexArrays(1, &renderData_->VAO_);
	glGenBuffers(1, &renderData_->VBO_);
	glGenBuffers(1, &renderData_->IBO_);

	glBindVertexArray(renderData_->VAO_);
		glBindBuffer(GL_ARRAY_BUFFER, renderData_->VBO_);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderData_->IBO_);
	glBindVertexArray(0);

	RenderData::shaderProgram_.useUniformPack(unifCommon);

	RenderData::shaderProgram_.onProgramReloaded.add([this](auto const& prog) {
		RenderData::shaderProgram_.setupVAO(renderData_->VAO_);
	});
	RenderData::shaderProgram_.setupVAO(renderData_->VAO_);
}

Water::~Water()
{
	if (renderData_)
		delete renderData_, renderData_ = nullptr;
}

void Water::setReflectionTexture(unsigned texId) {
	renderData_->textureReflection_2D_ = texId;
}

void Water::setRefractionTexture(unsigned texId_2D, unsigned texId_Cube) {
	renderData_->textureRefraction_ = texId_2D;
	renderData_->textureRefraction_Cube_ = texId_Cube;
}

void Water::validateParams(WaterParams const& p) {
	assertDbg(p.innerRadius > 0);
	assertDbg(p.outerExtent > 0);
	assertDbg(p.innerRadius > 1.f / p.vertexDensity);
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
	nVertices_ = rows * cols + 2 * nSkirtVerts;
	pVertices_ = (WaterVertex*)malloc(sizeof(WaterVertex) * nVertices_);

	glm::vec3 topleft {-params_.innerRadius, 0.f, -params_.innerRadius};
	float dx = params_.innerRadius * 2.f / (cols - 1);
	float dz = params_.innerRadius * 2.f / (rows - 1);
	float wTexW = 100.f;	// world width of water texture
	float wTexH = 100.f;	// world height of water texture
	// compute water vertices
	for (unsigned i=0; i<rows; i++)
		for (unsigned j=0; j<cols; j++) {
			glm::vec2 jitter(srandf() * 0.1, srandf() * 0.1);
			new(&pVertices_[i*rows + j]) WaterVertex {
				topleft + glm::vec3(dx * j + jitter.x, 0, dz * i + jitter.y),	// position
				0.f																// fog
			};
		}
	// compute skirt vertices
	for (unsigned i=0; i<nSkirtVerts; i++) {
		glm::vec2 jitter(srandf() * 0.1, srandf() * 0.1);
		float x = extentRadius * cosf(i*skirtVertSector) + jitter.x;
		float z = extentRadius * sinf(i*skirtVertSector) + jitter.y;
		new(&pVertices_[rows*cols+i]) WaterVertex {
			{ x, 0, z },														// position
			0.f //1.f																	// fog
		};

		x = (extentRadius + 50) * cosf(i*skirtVertSector) - jitter.x;
		z = (extentRadius + 50) * sinf(i*skirtVertSector) - jitter.y;
		new(&pVertices_[rows*cols+i + nSkirtVerts]) WaterVertex {
			{ x, 20, z },														// position
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
	if (!renderData_->shaderProgram_.isValid()) {
		return;
	}
	// configure backface culling
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	// set-up shader, vertex buffer and uniforms
	renderData_->shaderProgram_.begin();
	// set-up textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, renderData_->textureNormal_);
	glUniform1i(renderData_->iTexNormal, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, renderData_->textureReflection_2D_);
	glUniform1i(renderData_->iTexReflection_2D_, 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, renderData_->textureRefraction_);
	glUniform1i(renderData_->iTexRefraction_, 2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_CUBE_MAP, renderData_->textureRefraction_Cube_);
	glUniform1i(renderData_->iTexRefraction_Cube_, 3);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, renderData_->textureFoam);
	glUniform1i(renderData_->iTexFoam, 4);
	glBindVertexArray(renderData_->VAO_);
	// do the drawing
	glDrawElements(GL_TRIANGLES, triangles_.size() * 3, GL_UNSIGNED_INT, nullptr);
	// unbind stuff
	glBindVertexArray(0);
	renderData_->shaderProgram_.end();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
}

void Water::update(float dt) {
}

int Water::getNormalTexture() const {
	return renderData_->textureNormal_;
}
