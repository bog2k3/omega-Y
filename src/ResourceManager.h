#pragma once

#include <string>

class ResourceManager {
public:
	// retrieves (and loads if necessary) a texture id for a given image path.
	// [linearizeGamma] has effect only the first time the texture is loaded.
	static unsigned getTexture(std::string path, bool linearizeGamma=true);

	// TODO - similar methods for models, sounds etc

private:
	struct Data;
	static Data *pData_;
};
