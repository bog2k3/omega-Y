#ifndef UPACK_COMMON_H
#define UPACK_COMMON_H

#include <boglfw/renderOpenGL/UniformPack.h>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

// provides a uniform pack for "common" uniforms from common.glsl
class UPackCommon : public UniformPack {
public:
	UPackCommon();

	void setEyePos(glm::vec3 val)		{ setUniform(iEyePos, val); }
	void setEnableClipping(bool val)	{ setUniform(ibEnableClipping, val ? 1 : 0); }
	void setSubspace(float val)			{ setUniform(iSubspace, val); }
	void setbRefraction(bool val)		{ setUniform(ibRefraction, val ? 1 : 0); }
	void setbReflection(bool val)		{ setUniform(ibReflection, val ? 1 : 0); }
	void setTime(float val)				{ setUniform(iTime, val); }
	void setMatProjView(glm::mat4 val)	{ setUniform(iMatPV, val); }

private:
	unsigned iEyePos;
	unsigned ibEnableClipping;
	unsigned iSubspace;
	unsigned ibRefraction;
	unsigned ibReflection;
	unsigned iTime;
	unsigned iMatPV;
};

#endif // UPACK_COMMON_H
