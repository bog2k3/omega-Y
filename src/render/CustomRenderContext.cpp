#include "CustomRenderContext.h"

#include "programs/SharedUniformPacks.h"
#include "programs/UPackCommon.h"

#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Camera.h>

void CustomRenderContext::updateCommonUniforms() {
	assertDbg(SharedUniformPacks::upCommon && "Uniform pack not initialized!");
	auto unifCommon = SharedUniformPacks::upCommon;
	unifCommon->setbReflection(renderPass == RenderPass::WaterReflection);
	unifCommon->setbRefraction(renderPass == RenderPass::WaterRefraction);
	unifCommon->setEyePos(viewport().camera().position());
	unifCommon->setMatProjView(viewport().camera().matProjView());
	unifCommon->setEnableClipping(enableClipPlane);
	unifCommon->setSubspace(subspace);
	unifCommon->setTime(time);
}
