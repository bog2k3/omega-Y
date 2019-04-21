#ifndef UNIFORMPACK_H
#define UNIFORMPACK_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>

#include <vector>
#include <string>

enum class UniformType {
	INT,
	FLOAT,
	VEC2,
	iVEC2,
	VEC3,
	iVEC3,
	VEC4,
	iVEC4,
	MAT3,
	MAT4
};

// provides a collection of uniforms that can be shared accross multiple programs
class UniformPack
{
public:
	UniformPack() = default;
	virtual ~UniformPack() = default;

	struct UniformDescriptor {
		std::string name;
		UniformType type;
	};

	// adds a uniform description to the pack;
	// returns the index of the new uniform within this pack.
	unsigned addUniform(UniformDescriptor desc);

	// returns the number of uniforms held by this pack
	unsigned count() const { return elements_.size(); }
	// returns the ith uniform descriptor within this pack.
	UniformDescriptor const& element(unsigned i) { return elements_[i].descriptor; }

	// the following methods set values for the uniforms within this pack.
	// Note that these values are not sent to OpenGL yet, that only happens
	// when a ShaderProgram that uses this pack is being set up for rendering.

	void setUniform(unsigned indexInPack, int value);
	void setUniform(unsigned indexInPack, float value);
	void setUniform(unsigned indexInPack, glm::vec2 const& value);
	void setUniform(unsigned indexInPack, glm::ivec2 const& value);
	void setUniform(unsigned indexInPack, glm::vec3 const& value);
	void setUniform(unsigned indexInPack, glm::ivec3 const& value);
	void setUniform(unsigned indexInPack, glm::vec4 const& value);
	void setUniform(unsigned indexInPack, glm::ivec4 const& value);
	void setUniform(unsigned indexInPack, glm::mat3 const& value, bool transpose=false);
	void setUniform(unsigned indexInPack, glm::mat4 const& value, bool transpose=false);

	// pushes a uniform value from this pack into OpenGL's pipeline at the specified location.
	void pushValue(unsigned indexInPack, unsigned glLocation);

private:
	union UniformValue {
		int int_;
		float float_;
		glm::vec2 vec2_;
		glm::ivec2 ivec2_;
		glm::vec3 vec3_;
		glm::ivec3 ivec3_;
		glm::vec4 vec4_;
		glm::ivec4 ivec4_;
		glm::mat3 mat3_;
		glm::mat4 mat4_;
	};
	struct Element {
		UniformDescriptor descriptor;
		UniformValue value;
		bool transposed = false;
	};
	std::vector<Element> elements_;
};

#endif // UNIFORMPACK_H
