#include "ImgDebugDraw.h"

#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/utils/log.h>
#include <boglfw/renderOpenGL/glToolkit.h>

#include <GL/glew.h>

struct ImgDebugDraw::RenderData {
	int shaderProgram;
	int iPos;
	int iUV;
	int iSamp;
	int iRangeMin;
	int iRangeMax;
	int iChannelMask;
	unsigned VAO;
	unsigned VBO;
	unsigned IBO;
	unsigned texture;
	float rangeMin;
	float rangeMax;
	float channelMask[3];
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
"uniform vec3 channelMask;"
"void main() {"
	"vec3 texData = texture(tex0, fUV).xyz;"
	"vec3 adjusted = (texData - vec3(rangeMin, rangeMin, rangeMin)) / (rangeMax - rangeMin);\n"
	"float gamma = 2.2;"
	"adjusted.xyz = pow(adjusted.xyz, vec3(gamma));"
	"gl_FragColor = vec4(channelMask*adjusted, 1.0);"
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
	pRenderData_->iChannelMask = glGetUniformLocation(pRenderData_->shaderProgram, "channelMask");
	checkGLError("ImgDebugDraw shaders");

	glGenTextures(1, &pRenderData_->texture);
	glBindTexture(GL_TEXTURE_2D, pRenderData_->texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
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
	checkGLError("ImgDebugDraw buffers setup");
}

ImgDebugDraw::~ImgDebugDraw() {

}

#include <boglfw/utils/rand.h>

void ImgDebugDraw::setValues(const float* values, int width, int height, float rangeMin, float rangeMax, ImgDebugDraw::PixelFormat fmt) {
	pRenderData_->channelMask[0] = 0.f;
	pRenderData_->channelMask[1] = 0.f;
	pRenderData_->channelMask[2] = 0.f;
	unsigned glIntFmt;
	unsigned glFmt;
	switch (fmt) {
		case FMT_RGB:
			glIntFmt = GL_RGB32F;
			glFmt = GL_RGB;
			pRenderData_->channelMask[0] = 1.f;
			pRenderData_->channelMask[1] = 1.f;
			pRenderData_->channelMask[2] = 1.f;
			break;
		case FMT_XY:
			glIntFmt = GL_RG32F;
			glFmt = GL_RG;
			pRenderData_->channelMask[0] = 1.f;
			pRenderData_->channelMask[1] = 1.f;
			break;
		case FMT_GRAYSCALE:
			glIntFmt = GL_R32F;
			glFmt = GL_RED;
			pRenderData_->channelMask[0] = 1.f;
			break;
	}
	glBindTexture(GL_TEXTURE_2D, pRenderData_->texture);
	glTexImage2D(GL_TEXTURE_2D, 0, glIntFmt, width, height, 0, glFmt, GL_FLOAT, values);
	glBindTexture(GL_TEXTURE_2D, 0);
	pRenderData_->rangeMin = rangeMin;
	pRenderData_->rangeMax = rangeMax;
	checkGLError("ImgDebugDraw image data");
}

void ImgDebugDraw::draw(Viewport* vp) {
	if (!enabled_)
		return;
	glUseProgram(pRenderData_->shaderProgram);
	glBindVertexArray(pRenderData_->VAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, pRenderData_->texture);
	glUniform1i(pRenderData_->iSamp, 0);
	glUniform1f(pRenderData_->iRangeMin, pRenderData_->rangeMin);
	glUniform1f(pRenderData_->iRangeMax, pRenderData_->rangeMax);
	glUniform3fv(pRenderData_->iChannelMask, 1, pRenderData_->channelMask);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	checkGLError("ImgDebugDraw render");
}
