#include "ShaderProgramManager.h"

std::unordered_map<std::type_index, void*> ShaderProgramManager::shaderPrograms_;

void ShaderProgramManager::unloadAll() {
	for (auto &p : shaderPrograms_)
		if (p.second)
			delete p.second;
	shaderPrograms_.clear();
}
