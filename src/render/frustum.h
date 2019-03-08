#ifndef __FRUSTUM_H__
#define __FRUSTUM_H__

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

struct Frustum {
	enum PLANES {
		PLANE_NEAR = 0,
		PLANE_FAR,
		PLANE_LEFT,
		PLANE_RIGHT,
		PLANE_TOP,
		PLANE_BOTTOM,
	};
	// the 6 planes facing inward that define the view frustum;
	// use the enum above to index the array.
	glm::vec4 planes[6];

	enum CORNERS {
		CORNER_NLB = 0,	// Near Left Bottom
		CORNER_NLT,		// Near Left Top
		CORNER_NRT,		// Near Right Top
		CORNER_NRB,		// Near Right Bottom
		CORNER_FLB,		// Far Left Bottom
		CORNER_FLT,		// Far Left Top
		CORNER_FRT,		// Far Right Top
		CORNER_FRB		// Far Right Bottom
	};
	// the 8 corners (vertices) at the intersection of the frustum planes;
	// use the enum above to index the array.
	glm::vec3 corners[8];

	// the "view" origin - intersection point of left, right, top and bottom planes
	//glm::vec3 viewOrigin;

	// the central axis of the frustum - from view origin to the center of the far clipping plane
	glm::vec3 viewDirection;

	// initialize the frustum from a projection-view matrix
	Frustum(glm::mat4 const& projViewMatrix);
};

struct Trapezoid {
	// the points are in clockwise order
	union {
		struct {
			glm::vec3 P;
			glm::vec3 Q;
			glm::vec3 R;
			glm::vec3 S;
		} t;
		glm::vec3 v[4];
	};
};

// Projects a frustum onto a plane and returns the resulting trapezoid.
// The near and far clipping planes of the frustum are not taken into account;
// thus the four points of the trapezoid corespond to the points of intersection of the 4 edges of the frustum from the view origin.
// The intersections are treated only if they occur in front of the view origin of the frustum;
// if one or more of the axes don't intersect the plane in front of the view origin, their corresponding trapezoid points
// are assigned values according to the [maxDistance] parameter - see below.
// The points of the trapezoid correspond to these edges:
// 0/P = bottom right
// 1/Q = bottom left
// 2/R = top left
// 3/S = top right
// The [maxDistance] parameter controls where the points will be generated if no intersection occurs - in this case
// the corresponding point will be generated on the plane, at [maxDistance] distance from the plane-projected view origin,
// in the plane-projected direction of its coresponding edge.
Trapezoid projectFrustum(Frustum const& f, glm::vec4 const& plane, float maxDistance);

#endif // __FRUSTUM_H__
