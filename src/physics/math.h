#ifndef __PHYSICS_MATH_H__
#define __PHYSICS_MATH_H__

// this header contains functions to transform data types between Bullet physics engine and GLM

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <bullet3/LinearMath/btVector3.h>

// Bullet to GLM functions -----------------------------------------------------------

inline glm::vec3 b2g(btVector3 const& v) {
	return glm::vec3{v.x(), v.y(), v.z()};
}

// GLM to Bullet functions -----------------------------------------------------------

inline btVector3 g2b(glm::vec3 const& v) {
	return btVector3{v.x, v.y, v.z};
}

#endif // __PHYSICS_MATH_H__
