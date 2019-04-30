#include "SkyBox.h"
#include "../render/CustomRenderContext.h"
#include "../render/ShaderProgramManager.h"
#include "../render/programs/ShaderSkybox.h"
#include "../render/programs/UPackSkybox.h"

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
	ShaderSkybox* shaderProgram;
	int reloadHandler;
	unsigned texture;
};

struct SkyBoxVertex {
	glm::vec3 pos;
};

void SkyBox::setupVAO() {
	glBindVertexArray(renderData_->VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderData_->IBO);

	std::map<std::string, ShaderProgram::VertexAttribSource> mapVertexSources {
		{ "pos", { renderData_->VBO, sizeof(SkyBoxVertex), offsetof(SkyBoxVertex, pos) } },
	};
	renderData_->shaderProgram->setupVertexStreams(mapVertexSources);

	glBindVertexArray(0);
}

SkyBox::SkyBox() {
	LOGPREFIX("SkyBox");
	renderData_ = new SkyBoxRenderData();
	glGenVertexArrays(1, &renderData_->VAO);
	glGenBuffers(1, &renderData_->VBO);
	glGenBuffers(1, &renderData_->IBO);

	renderData_->shaderProgram = &ShaderProgramManager::requestProgram<ShaderSkybox>();
	renderData_->reloadHandler = renderData_->shaderProgram->onProgramReloaded.add([this](auto const&) {
		setupVAO();
	});
	setupVAO();

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
	renderData_->shaderProgram->onProgramReloaded.remove(renderData_->reloadHandler);
	clear();
	glDeleteBuffers(1, &renderData_->VBO);
	glDeleteBuffers(1, &renderData_->IBO);
	glDeleteVertexArrays(1, &renderData_->VAO);
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
	glBindVertexArray(renderData_->VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, renderData_->texture);
	renderData_->shaderProgram->uniforms().setSkyboxSampler(0);
	renderData_->shaderProgram->begin();

	int oldDepthMask;
	glGetIntegerv(GL_DEPTH_WRITEMASK, &oldDepthMask);
	glDepthMask(GL_FALSE);	// disable depth buffer writing

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);

	glDepthMask(oldDepthMask);	// enable depth buffer writing

	renderData_->shaderProgram->end();
	glBindVertexArray(0);
}

void SkyBox::update(float dt) {

}

unsigned SkyBox::getCubeMapTexture() const {
	return renderData_->texture;
}
