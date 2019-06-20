#include "ResourceManager.h"

#include <boglfw/utils/assert.h>
#include <boglfw/renderOpenGL/TextureLoader.h>

#include <map>
#include <mutex>

struct rcDescriptor {
	enum _type {
		RC_TEXTURE,
		RC_MODEL,
		RC_SOUND
	} type;
	unsigned texId;

	void loadTexture(std::string const& path, bool linearizeValues) {
		type = RC_TEXTURE;
		texId = TextureLoader::loadFromPNG(path, linearizeValues);
	}
	void loadModel(const char* path);
	void loadSound(const char* path);
};

struct ResourceManager::Data {
	std::map<std::string, rcDescriptor> cache;
	std::mutex mtx;
};

ResourceManager::Data* ResourceManager::pData_ = new ResourceManager::Data();

unsigned ResourceManager::getTexture(std::string path, bool linearizeGamma) {
	assertDbg(pData_);
	decltype(Data::cache)::iterator it = pData_->cache.end();
	std::unique_lock<std::mutex> lk(pData_->mtx);
	it = pData_->cache.find(path);
	lk.unlock();
	if (it == pData_->cache.end()) {
		rcDescriptor rcdesc;
		rcdesc.loadTexture(path, linearizeGamma);
		lk.lock();
		it = pData_->cache.insert({path, rcdesc}).first;
		lk.unlock();
	}
	assertDbg(it != pData_->cache.end());
	return it->second.texId;
}
