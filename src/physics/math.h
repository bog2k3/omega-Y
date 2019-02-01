#ifndef __PHYSICS_MATH_H__
#define __PHYSICS_MATH_H__

// this header contains functions to transform data types between Bullet physics engine and GLM

#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>

#include <bullet3/LinearMath/btVector3.h>
#include <bullet3/LinearMath/btQuaternion.h>

// Bullet to GLM functions -----------------------------------------------------------

inline glm::vec3 b2g(btVector3 const& v) {
	return glm::vec3{v.x(), v.y(), v.z()};
}

inline glm::quat b2g(btQuaternion const& q) {
	return glm::quat{q.x(), q.y(), q.z(), q.w()};
}

// GLM to Bullet functions -----------------------------------------------------------

inline btVector3 g2b(glm::vec3 const& v) {
	return btVector3{v.x, v.y, v.z};
}

inline btQuaternion g2b(glm::quat const& q) {
	return btQuaternion{q.x, q.y, q.z, q.w};
}

#endif // __PHYSICS_MATH_H__
