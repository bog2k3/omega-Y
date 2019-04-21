#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include "UniformPackProxy.h"

#include <memory>
#include <vector>

class UniformPack;

class ShaderProgram
{
public:
	ShaderProgram();
	virtual ~ShaderProgram();

	// Assigns a uniform pack to be used by this program.
	// This can be called multiple times with different packs, all of which will be used.
	// The method must be called before loading the program.
	void assignUniformPack(std::shared_ptr<UniformPack> pack);

	// loads and compiles the shaders, then links the program and fetches all uniform locations that have been mapped
	void load(std::string const& vertPath, std::string const& fragPath, std::string const& geomPath="");

	// sets this program up for rendering. This will also push all uniform values from all
	// assigned uniform packs into the openGL pipeline
	void begin();

	// resets the openGL state after you finished rendering with this program.
	void end();

protected:
	std::vector<UniformPackProxy> uniformPackProxies_;
};

#endif // SHADERPROGRAM_H
