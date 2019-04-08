#include "frustum.h"

#include <boglfw/math/math3D.h>

Frustum::Frustum(glm::mat4 const& projViewMatrix) {
	glm::mat4 mInvPV = glm::inverse(projViewMatrix);

	glm::vec4 homogenousCorners[8] {
		{-1.f, -1.f, -1.f, 1.f},
		{-1.f, +1.f, -1.f, 1.f},
		{+1.f, +1.f, -1.f, 1.f},
		{+1.f, -1.f, -1.f, 1.f},
		{-1.f, -1.f, +1.f, 1.f},
		{-1.f, +1.f, +1.f, 1.f},
		{+1.f, +1.f, +1.f, 1.f},
		{+1.f, -1.f, +1.f, 1.f}
	};
	for (int i=0; i<8; i++) {
		glm::vec4 v = mInvPV * homogenousCorners[i];
		corners[i] = vec4xyz(v) / v.w;
	}

	viewDirection = glm::normalize(vec4xyz(mInvPV * glm::vec4(0, 0, 1, 0)));
	//viewOrigin

	planes[PLANE_NEAR] = planeFromPoints(corners[CORNER_NRB], corners[CORNER_NRT], corners[CORNER_NLT]);
	planes[PLANE_FAR] = planeFromPoints(corners[CORNER_FLB], corners[CORNER_FLT], corners[CORNER_FRT]);
	planes[PLANE_LEFT] = planeFromPoints(corners[CORNER_NLB], corners[CORNER_NLT], corners[CORNER_FLT]);
	planes[PLANE_RIGHT] = planeFromPoints(corners[CORNER_NRT], corners[CORNER_NRB], corners[CORNER_FRB]);
	planes[PLANE_TOP] = planeFromPoints(corners[CORNER_NLT], corners[CORNER_NRT], corners[CORNER_FRT]);
	planes[PLANE_BOTTOM] = planeFromPoints(corners[CORNER_NRB], corners[CORNER_NLB], corners[CORNER_FLB]);
}

Trapezoid projectFrustum(Frustum const& f, glm::vec4 const& plane, float maxDistance) {
	Trapezoid t;
	std::pair<glm::vec3, glm::vec3> frustumEdges[4] {
		{ f.corners[Frustum::CORNER_NRB], glm::normalize(f.corners[Frustum::CORNER_FRB] - f.corners[Frustum::CORNER_NRB]) },
		{ f.corners[Frustum::CORNER_NLB], glm::normalize(f.corners[Frustum::CORNER_FLB] - f.corners[Frustum::CORNER_NLB]) },
		{ f.corners[Frustum::CORNER_NLT], glm::normalize(f.corners[Frustum::CORNER_FLT] - f.corners[Frustum::CORNER_NLT]) },
		{ f.corners[Frustum::CORNER_NRT], glm::normalize(f.corners[Frustum::CORNER_FRT] - f.corners[Frustum::CORNER_NRT]) },
	};
	glm::vec3 planeNormal = vec4xyz(plane);
	for (int i=0; i<4; i++) {
		std::pair<glm::vec4, bool> I = rayIntersectPlane(frustumEdges[i].first, frustumEdges[i].second, plane);
		glm::vec3& dir = frustumEdges[i].second;
		// project direction onto plane:
		glm::vec3 dirOnPlane = glm::normalize(dir - planeNormal * glm::dot(dir, planeNormal));
		float projDistance = I.first.w * dot(dir, dirOnPlane);
		if (I.second && I.first.w > 0 && projDistance <= maxDistance) { // TODO project distance onto plane before comparing
			// edge intersects the plane in front of the frustum and before maxDistance
			t.v[i] = vec4xyz(I.first);
		} else {
			// edge doesn't intersect the plane in front of the frustum or at all
			glm::vec3& corner = frustumEdges[i].first;
			// project corner onto plane:
			glm::vec3 cornerOnPlane = corner - planeNormal * pointDotPlane(corner, plane);
			// compute point at maxDist:
			t.v[i] = cornerOnPlane + dirOnPlane * maxDistance;
		}
	}
	return t;
}
