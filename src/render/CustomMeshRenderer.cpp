#include "CustomMeshRenderer.h"
#include "CustomRenderContext.h"

#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Mesh.h>
#include <boglfw/renderOpenGL/shader.h>
#include <boglfw/renderOpenGL/Camera.h>
#include <boglfw/renderOpenGL/glToolkit.h>
#include <boglfw/utils/log.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GL/gl.h>

struct CustomMeshRenderer::RenderData {
	unsigned shaderProgram_;

	int iPos;
	int iNorm;
	int iUV;
	int iColor;
	int imPV;
	int imW;

	int iEyePos;
	int iSubspace;
	int ibRefraction;
	int ibReflection;
	int iTime;
};

CustomMeshRenderer::CustomMeshRenderer()
	: pRenderData_(new RenderData()) {
	LOGPREFIX("CustomMeshRenderer::ctor");
	Shaders::createProgram("data/shaders/mesh-custom.vert", "data/shaders/mesh-custom.frag", [this](unsigned id) {
		pRenderData_->shaderProgram_ = id;
		if (pRenderData_->shaderProgram_ == 0) {
			throw std::runtime_error("Unable to load custom mesh shaders!!");
		}
		pRenderData_->iPos = glGetAttribLocation(pRenderData_->shaderProgram_, "pos");
		pRenderData_->iNorm = glGetAttribLocation(pRenderData_->shaderProgram_, "normal");
		pRenderData_->iUV = glGetAttribLocation(pRenderData_->shaderProgram_, "uv");
		pRenderData_->iColor = glGetAttribLocation(pRenderData_->shaderProgram_, "color");
		pRenderData_->imPV = glGetUniformLocation(pRenderData_->shaderProgram_, "mPV");
		pRenderData_->imW = glGetUniformLocation(pRenderData_->shaderProgram_, "mW");
		pRenderData_->iEyePos = glGetUniformLocation(pRenderData_->shaderProgram_, "eyePos");
		pRenderData_->iSubspace = glGetUniformLocation(pRenderData_->shaderProgram_, "subspace");
		pRenderData_->ibRefraction = glGetUniformLocation(pRenderData_->shaderProgram_, "bRefraction");
		pRenderData_->ibReflection = glGetUniformLocation(pRenderData_->shaderProgram_, "bReflection");
		pRenderData_->iTime = glGetUniformLocation(pRenderData_->shaderProgram_, "time");
		checkGLError("getAttribs");
	});
}

CustomMeshRenderer::~CustomMeshRenderer()
{
	if (pRenderData_->shaderProgram_)
		glDeleteProgram(pRenderData_->shaderProgram_);
	delete pRenderData_, pRenderData_ = nullptr;
}

void CustomMeshRenderer::renderMesh(Mesh& mesh, glm::mat4 const& matW, RenderContext const& ctx) {
	LOGPREFIX("CustomMeshRenderer::renderMesh");

	if (!pRenderData_->shaderProgram_)
		return;

	auto const& rctx = CustomRenderContext::fromCtx(ctx);

	glUseProgram(pRenderData_->shaderProgram_);

	auto matPV = rctx.viewport.camera().matProjView();
	glUniformMatrix4fv(pRenderData_->imPV, 1, GL_FALSE, glm::value_ptr(matPV));
	glUniformMatrix4fv(pRenderData_->imW, 1, GL_FALSE, glm::value_ptr(matW));

	checkGLError("uniforms setup 1");

	if (pRenderData_->iEyePos >= 0)
		glUniform3fv(pRenderData_->iEyePos, 1, &ctx.viewport.camera().position().x);
	if (pRenderData_->iSubspace >= 0)
		glUniform1f(pRenderData_->iSubspace, rctx.clipPlane.y);
	if (pRenderData_->ibRefraction >= 0)
		glUniform1i(pRenderData_->ibRefraction, rctx.renderPass == RenderPass::WaterRefraction ? 1 : 0);
	if (pRenderData_->ibReflection >= 0)
		glUniform1i(pRenderData_->ibReflection, rctx.renderPass == RenderPass::WaterReflection ? 1 : 0);
	if (pRenderData_->iTime >= 0)
		glUniform1f(pRenderData_->iTime, rctx.time);

	checkGLError("uniforms setup 2");

	if (rctx.enableClipPlane)
		glEnable(GL_CLIP_DISTANCE0);

	glBindVertexArray(mesh.getVAO());
	if (mesh.vertexAttribsProgramBinding_ != pRenderData_->shaderProgram_) {
		glBindBuffer(GL_ARRAY_BUFFER, mesh.getVBO());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.getIBO());
		glEnableVertexAttribArray(pRenderData_->iPos);
		glEnableVertexAttribArray(pRenderData_->iNorm);
		glEnableVertexAttribArray(pRenderData_->iUV);
		glEnableVertexAttribArray(pRenderData_->iColor);
		glVertexAttribPointer(pRenderData_->iPos, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::s_Vertex), (void*)offsetof(Mesh::s_Vertex, position));
		glVertexAttribPointer(pRenderData_->iNorm, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::s_Vertex), (void*)offsetof(Mesh::s_Vertex, normal));
		glVertexAttribPointer(pRenderData_->iUV, 2, GL_FLOAT, GL_FALSE, sizeof(Mesh::s_Vertex), (void*)offsetof(Mesh::s_Vertex, UV1));
		glVertexAttribPointer(pRenderData_->iColor, 4, GL_FLOAT, GL_FALSE, sizeof(Mesh::s_Vertex), (void*)offsetof(Mesh::s_Vertex, color));
		mesh.vertexAttribsProgramBinding_ = pRenderData_->shaderProgram_;
		checkGLError("attrib arrays setup");
	}
	// decide what to draw:
	unsigned drawMode = 0;
	switch (mesh.getRenderMode()) {
		case Mesh::RENDER_MODE_POINTS:
			drawMode = GL_POINTS; break;
		case Mesh::RENDER_MODE_LINES:
			drawMode = GL_LINES; break;
		case Mesh::RENDER_MODE_TRIANGLES:
		case Mesh::RENDER_MODE_TRIANGLES_WIREFRAME:
			drawMode = GL_TRIANGLES; break;
		default:
			assert(false && "Unknown mesh draw mode!");
	}
	if (mesh.getRenderMode() == Mesh::RENDER_MODE_TRIANGLES_WIREFRAME || mesh.getRenderMode() == Mesh::RENDER_MODE_LINES) {
		glLineWidth(2.f);
	}
	if (mesh.getRenderMode() == Mesh::RENDER_MODE_TRIANGLES_WIREFRAME) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	glDrawElements(drawMode, mesh.getElementsCount(), GL_UNSIGNED_SHORT, 0);
	checkGLError("mesh draw");
	glBindVertexArray(0);
	if (mesh.getRenderMode() == Mesh::RENDER_MODE_TRIANGLES_WIREFRAME || mesh.getRenderMode() == Mesh::RENDER_MODE_LINES) {
		glLineWidth(1.f);
	}
	if (mesh.getRenderMode() == Mesh::RENDER_MODE_TRIANGLES_WIREFRAME) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	glDisable(GL_CLIP_DISTANCE0);
}

