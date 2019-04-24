#include "CustomRenderContext.h"

#include "programs/UniformPackCollection.h"
#include "programs/UPackCommon.h"

#include <boglfw/renderOpenGL/Viewport.h>
#include <boglfw/renderOpenGL/Camera.h>

void CustomRenderContext::updateCommonUniforms() {
	assertDbg(UniformPackCollection::upCommon && "Uniform pack not initialized!");
	auto unifCommon = UniformPackCollection::upCommon;
	unifCommon->setbReflection(renderPass == RenderPass::WaterReflection);
	unifCommon->setbRefraction(renderPass == RenderPass::WaterRefraction);
	unifCommon->setEyePos(viewport().camera().position());
	unifCommon->setMatProjView(viewport().camera().matProjView());
	unifCommon->setEnableClipping(enableClipPlane);
	unifCommon->setSubspace(subspace);
	unifCommon->setTime(time);
}
