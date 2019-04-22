#include "UPackCommon.h"

UPackCommon::UPackCommon() {
	iEyePos = addUniform({"eyePos", UniformType::VEC3});
	iSubspace = addUniform({"subspace", UniformType::FLOAT});
	ibRefraction = addUniform({"bRefraction", UniformType::INT});
	ibReflection = addUniform({"bReflection", UniformType::INT});
	iTime = addUniform({"time", UniformType::FLOAT});
	iMatPV = addUniform({"matPV", UniformType::MAT4});
}
