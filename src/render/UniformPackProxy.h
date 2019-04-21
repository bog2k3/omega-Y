#ifndef UNIFORM_PACK_PROXY_H
#define UNIFORM_PACK_PROXY_H

#include <memory>
#include <vector>

class UniformPack;

class UniformPackProxy {
public:
	UniformPackProxy(std::shared_ptr<UniformPack> pack);

	// updates the internal mappings between the uniform pack and the opengl program.
	void updateMappings(unsigned programId);

	// pushes all uniform values from the uniform pack into corresponding openGL's uniform locations
	void pushValues();

	const UniformPack& getPack() const { return *pack_.get(); }

private:
	std::shared_ptr<UniformPack> pack_;	// pointer to the actual pack
	std::vector<int> uniformIndexes_;	// maps each uniform from the pack to the actual uniform location in the program
};

#endif // UNIFORM_PACK_PROXY_H
