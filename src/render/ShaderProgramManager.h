#pragma once

#include <boglfw/renderOpenGL/ShaderProgram.h>

#include <unordered_map>
#include <typeindex>
#include <type_traits>

class ShaderProgramManager {
public:
	template<class C>
	static C& requestProgram() {
		static_assert(std::is_base_of<ShaderProgram, C>::value && "Only classes derived from ShaderProgram can be requested");
		auto it = shaderPrograms_.find(typeid(C));
		if (it == shaderPrograms_.end()) {
			// create the program here
			C* c = new C();
			c->load();
			shaderPrograms_[typeid(C)] = (void*)c;
			return *c;
		} else
			return *(C*)(it->second);
	}

	static void unloadAll();

private:
	static std::unordered_map<std::type_index, void*> shaderPrograms_;
};
