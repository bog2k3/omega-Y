#include "SkyBox.h"
#include "../render/CustomRenderContext.h"

#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/renderOpenGL/TextureLoader.h>
#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/utils/log.h>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>

struct SkyBox::SkyBoxRenderData {
	unsigned VAO;
	unsigned VBO;
	unsigned IBO;
	unsigned shaderProgram;
	unsigned iPos;
	unsigned iMatVP;
	unsigned iTexSampler;
	unsigned texture;
};

struct SkyBoxVertex {
	glm::vec3 pos;
};

SkyBox::SkyBox() {
	LOGPREFIX("SkyBox");
	renderData_ = new SkyBoxRenderData();
	glGenVertexArrays(1, &renderData_->VAO);
	glGenBuffers(1, &renderData_->VBO);
	glGenBuffers(1, &renderData_->IBO);

	Shaders::createProgram("data/shaders/skybox.vert", "data/shaders/skybox.frag", [this](unsigned id) {
		renderData_->shaderProgram = id;
		if (!renderData_->shaderProgram) {
			ERROR("Could not load skybox shaders!");
			throw;
		}
		renderData_->iPos = glGetAttribLocation(renderData_->shaderProgram, "pos");
		renderData_->iMatVP = glGetUniformLocation(renderData_->shaderProgram, "mVP");
		renderData_->iTexSampler = glGetUniformLocation(renderData_->shaderProgram, "textureSky");

		glBindVertexArray(renderData_->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, renderData_->VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderData_->IBO);
		glEnableVertexAttribArray(renderData_->iPos);
		glVertexAttribPointer(renderData_->iPos, 3, GL_FLOAT, GL_FALSE, sizeof(SkyBoxVertex), (void*)offsetof(SkyBoxVertex, pos));
		glBindVertexArray(0);
	});

	// generate vertex data:
	SkyBoxVertex verts[] {
		// front face
		{{-1.f, -1.f, +1.f}},	// bottom left
		{{-1.f, +1.f, +1.f}},	// top left
		{{+1.f, +1.f, +1.f}},	// top right
		{{+1.f, -1.f, +1.f}},	// bottom right
		// left face
		{{-1.f, -1.f, -1.f}},	// bottom left
		{{-1.f, +1.f, -1.f}},	// top left
		{{-1.f, +1.f, +1.f}},	// top right
		{{-1.f, -1.f, +1.f}},	// bottom right
		// right face
		{{+1.f, -1.f, +1.f}},	// bottom left
		{{+1.f, +1.f, +1.f}},	// top left
		{{+1.f, +1.f, -1.f}},	// top right
		{{+1.f, -1.f, -1.f}},	// bottom right
		// back face
		{{+1.f, -1.f, -1.f}},	// bottom left
		{{+1.f, +1.f, -1.f}},	// top left
		{{-1.f, +1.f, -1.f}},	// top right
		{{-1.f, -1.f, -1.f}},	// bottom right
		// top face
		{{-1.f, +1.f, +1.f}},	// bottom left
		{{-1.f, +1.f, -1.f}},	// top left
		{{+1.f, +1.f, -1.f}},	// top right
		{{+1.f, +1.f, +1.f}},	// bottom right
		// bottom face
		{{-1.f, -1.f, -1.f}},	// bottom left
		{{-1.f, -1.f, +1.f}},	// top left
		{{+1.f, -1.f, +1.f}},	// top right
		{{+1.f, -1.f, -1.f}},	// bottom right
	};
	glBindBuffer(GL_ARRAY_BUFFER, renderData_->VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), (void*)verts, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// generate index data
	uint16_t inds[] {0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 8, 9, 10, 8, 10, 11, 12,
					13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23};
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderData_->IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), (void*)inds, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

SkyBox::~SkyBox() {
	clear();
	delete renderData_, renderData_ = nullptr;
}

// loads a static skybox from a path that contains ./front.png, ./left.png, ./right.png, ./back.png, ./top.png, ./bottom.png
void SkyBox::load(std::string const& path) {
	LOGPREFIX("SkyBox::load");
	clear();
	std::string filenames[6] {
		path + "/right.png",	//X+
		path + "/left.png",		//X-
		path + "/top.png",		//Y+
		path + "/bottom.png",	//Y-
		path + "/front.png",	//Z+
		path + "/back.png",		//Z-
	};
	renderData_->texture = TextureLoader::loadCubeFromPNG(filenames, true);
	if (!renderData_->texture) {
		ERROR("Failed to load skybox textures from " << path);
		throw;
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, renderData_->texture);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

// generates a dynamic skybox
void SkyBox::generate(SkyBoxGenerationParams params) {
	clear();
}

// generates dynamic clouds
void SkyBox::generateClouds(CloudsGenerationParams params) {

}

void SkyBox::clear() {
	if (renderData_->texture)
		glDeleteTextures(1, &renderData_->texture);
	renderData_->texture = 0;
}

void SkyBox::draw(RenderContext const& ctx) {
	glUseProgram(renderData_->shaderProgram);
	glBindVertexArray(renderData_->VAO);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(renderData_->iTexSampler, 0);

	glm::mat4 mV = ctx.viewport().camera().matView();
	mV[3][0] = mV[3][1] = mV[3][2] = 0.f;	// reset translation to center the skybox on the camera
	glm::mat4 mPV = ctx.viewport().camera().matProj() * mV;

	glUniformMatrix4fv(renderData_->iMatVP, 1, GL_FALSE, glm::value_ptr(mPV));

	glDepthMask(GL_FALSE);	// disable depth buffer writing

	glBindTexture(GL_TEXTURE_CUBE_MAP, renderData_->texture);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);

	glDepthMask(GL_TRUE);	// enable depth buffer writing

	glBindVertexArray(0);
	glUseProgram(0);
}

void SkyBox::update(float dt) {

}

unsigned SkyBox::getCubeMapTexture() const {
	return renderData_->texture;
}
