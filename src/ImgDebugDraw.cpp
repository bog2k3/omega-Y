#include "ImgDebugDraw.h"

#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/utils/log.h>

#include <GL/glew.h>

struct ImgDebugDraw::RenderData {
	int shaderProgram;
	int iPos;
	int iUV;
	int iSamp;
	int iRangeMin;
	int iRangeMax;
	unsigned VAO;
	unsigned VBO;
	unsigned IBO;
	unsigned texture;
	float rangeMin;
	float rangeMax;
	float vertexData[5*4] {
		-1.f, -1.f, 0.f, 0.f, 0.f,	// bottom left
		-1.f, +1.f, 0.f, 0.f, 1.f,	// top left
		+1.f, +1.f, 0.f, 1.f, 1.f,	// top right
		+1.f, -1.f, 0.f, 1.f, 0.f,	// bottom right
	};
	uint16_t indexData[6] {
		0, 1, 2, 0, 2, 3
	};
};

static char vertex_shader[] = ""
"#version 330 core\n"
"in vec3 pos;"
"in vec2 uv;"
"out vec2 fUV;"
"void main() {"
	"gl_Position = vec4(pos, 1);"
	"fUV = uv;"
"}"
;

static char fragment_shader[] = ""
"#version 330 core\n"
"in vec2 fUV;"
"uniform sampler2D tex0;"
"uniform float rangeMin;"
"uniform float rangeMax;"
"void main() {"
	"vec3 texData = texture(tex0, fUV).xyz;"
	"vec3 adjusted = (texData - vec3(rangeMin, rangeMin, rangeMin)) / (rangeMax - rangeMin);"
	"gl_FragColor = vec4(adjusted, 1.0);"
"}"
;

ImgDebugDraw::ImgDebugDraw() {
	LOGPREFIX("ImgDebugDraw");
	unsigned vertShd = Shaders::createAndCompileShader(vertex_shader, GL_VERTEX_SHADER);
	unsigned fragShd = Shaders::createAndCompileShader(fragment_shader, GL_FRAGMENT_SHADER);
	if (!vertShd || !fragShd) {
		ERROR("Failed to compile shaders.");
		throw;
	}
	pRenderData_ = new RenderData();
	pRenderData_->shaderProgram = Shaders::linkProgram(vertShd, fragShd);
	if (!pRenderData_->shaderProgram) {
		ERROR("Failed to link shader program.");
		throw;
	}
	pRenderData_->iPos = glGetAttribLocation(pRenderData_->shaderProgram, "pos");
	pRenderData_->iUV = glGetAttribLocation(pRenderData_->shaderProgram, "uv");
	pRenderData_->iSamp = glGetUniformLocation(pRenderData_->shaderProgram, "tex0");
	pRenderData_->iRangeMin = glGetUniformLocation(pRenderData_->shaderProgram, "rangeMin");
	pRenderData_->iRangeMax = glGetUniformLocation(pRenderData_->shaderProgram, "rangeMax");
	glGenTextures(1, &pRenderData_->texture);
	glGenVertexArrays(1, &pRenderData_->VAO);
	glGenBuffers(1, &pRenderData_->VBO);
	glGenBuffers(1, &pRenderData_->IBO);

	glBindVertexArray(pRenderData_->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, pRenderData_->VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pRenderData_->IBO);
	glEnableVertexAttribArray(pRenderData_->iPos);
	glEnableVertexAttribArray(pRenderData_->iUV);
	glVertexAttribPointer(pRenderData_->iPos, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);
	glVertexAttribPointer(pRenderData_->iUV, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
	glBufferData(GL_ARRAY_BUFFER, sizeof(pRenderData_->vertexData), pRenderData_->vertexData, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pRenderData_->indexData), pRenderData_->indexData, GL_STATIC_DRAW);
	glBindVertexArray(0);
}

ImgDebugDraw::~ImgDebugDraw() {

}

void ImgDebugDraw::setValues(const float* values, int width, int height, float rangeMin, float rangeMax, ImgDebugDraw::PixelFormat fmt) {
	unsigned glFmt;
	switch (fmt) {
		case FMT_RGB:
			glFmt = GL_RGB32F;
			break;
		case FMT_XY:
			glFmt = GL_RG32F;
			break;
		case FMT_GRAYSCALE:
			glFmt = GL_R32F;
	}
	glBindTexture(GL_TEXTURE_2D, pRenderData_->texture);
	glTexImage2D(GL_TEXTURE_2D, 0, glFmt, width, height, 0, glFmt, GL_FLOAT, values);
	glBindTexture(GL_TEXTURE_2D, 0);
	pRenderData_->rangeMin = rangeMin;
	pRenderData_->rangeMax = rangeMax;
}

void ImgDebugDraw::draw(Viewport* vp) {
	if (!enabled_)
		return;
	glBindVertexArray(pRenderData_->VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pRenderData_->texture);
	glUniform1i(pRenderData_->iSamp, 0);
	glUniform1f(pRenderData_->iRangeMin, pRenderData_->rangeMin);
	glUniform1f(pRenderData_->iRangeMax, pRenderData_->rangeMax);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
}
