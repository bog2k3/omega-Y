#ifndef __PHYSICS_MATH_H__
#define __PHYSICS_MATH_H__

// this header contains functions to transform data types between Bullet physics engine and GLM

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

// Bullet to GLM functions -----------------------------------------------------------

glm::vec3 b2g(btVector3 const& v) {
	return glm::vec3{v.x(), v.y(), v.z()};
}

glm::vec4 b2g(btVector4 const& v) {
	return glm::vec4{v.x(), v.y(), v.z(), v.w()};
}

// GLM to Bullet functions -----------------------------------------------------------

btVector3 g2b(glm::vec3 const& v) {
	return btVector3{v.x, v.y, v.z};
}

btVector3 g2b(glm::vec4 const& v) {
	return btVector4{v.x, v.y, v.z, v.w};
}

#endif // __PHYSICS_MATH_H__