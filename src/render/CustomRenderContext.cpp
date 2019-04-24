#include "CustomRenderContext.h"

#include "programs/UPackCommon.h"

#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Camera.h>

CustomRenderContext::CustomRenderContext() {
	unifCommon = std::make_shared<UPackCommon>();
}

void CustomRenderContext::updateCommonUniforms() {
	unifCommon->setbReflection(renderPass == RenderPass::WaterReflection);
	unifCommon->setbRefraction(renderPass == RenderPass::WaterRefraction);
	unifCommon->setEyePos(viewport().camera().position());
	unifCommon->setMatProjView(viewport().camera().matProjView());
	unifCommon->setEnableClipping(enableClipPlane);
	unifCommon->setSubspace(subspace);
	unifCommon->setTime(time);
}
