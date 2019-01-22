#ifndef __TRIANGULATION_H__
#define __TRIANGULATION_H__

/* copyright 2016 Dr David Sinclair
   david@s-hull.org
 
   program to compute Delaunay triangulation of a set of points.

   this code is released under GPL3, 
   a copy ofthe license can be found at
   http://www.gnu.org/licenses/gpl-3.0.html

   you can purchase a un-restricted licnese from 
   http://www.s-hull.org 
   for the price of one beer!

   revised 12/feb/2016
 
 */
 
#include <glm/vec2.hpp>

#include <vector>

struct Triangle {
	int iV1, iV2, iV3;		// index of 1st, 2nd and 3rd vertices
	int iN12, iN13, iN23;	// index of neighbour Triangle at edge 1-2, 1-3 and 2-3
};


// Perform Delaunay triangulation of a set of points.
// returns 1 on success, negative error code on error.
// VERTEX_TYPE is any type for which the nth_elem<> template function is defined -- see below
// input: [points] array of points
// output: [triangles] vector of Triangle structure that describes the indexes of each vertex of each triangle and the index of the neighbour Triangle at each side
template<class VERTEX_TYPE>
int triangulate(VERTEX_TYPE* points, unsigned nPoints, std::vector<Triangle> &triangles);

// return the n-th component of the vertex v.
// usually, 0th element is .x, 1th is .y
template<class VERTEX_TYPE>
inline float nth_elem(VERTEX_TYPE const& v, unsigned n);

// common specialization
template <>
inline float nth_elem(glm::vec2 const& v, unsigned n) { return n==0 ? v.x : n==1 ? v.y : 0.f; }

// -------------- IMPLEMENTATION DETAILS FOLLOW -------------------------- //

struct Triad
{
	int a, b, c;
	int ab, bc, ac;  // adjacent edges index to neighbouring triangle.
	float ro, R, C;
	Triad() {}
	Triad(int x, int y) : a(x), b(y),c(0), ab(-1), bc(-1), ac(-1), ro(-1), R(0), C(0) {}
	Triad(int x, int y, int z) : a(x), b(y), c(z),  ab(-1), bc(-1), ac(-1), ro(-1), R(0), C(0) {}
	Triad(const Triad &p) : a(p.a), b(p.b), c(p.c), ab(p.ab), bc(p.bc), ac(p.ac), ro(p.ro), R(p.R), C(p.C) {}

	Triad &operator=(const Triad &p) {
		a = p.a;
		b = p.b;
		c = p.c;

		ab = p.ab;
		bc = p.bc;
		ac = p.ac;

		ro = p.ro;
		R = p.R;
		C = p.C;

		return *this;
	}
};


/* point structure for triangulate.
   has to keep track of triangle ids as hull evolves.
*/
struct Shx
{
	int id, trid;
	float r, c, tr, tc;
	float ro;
	Shx() {}
	Shx(float a, float b) : id(-1), trid(-1), r(a), c(b), tr(0.0), tc(0.0), ro(0.0) {}
	Shx(float a, float b, float x) : id(-1), trid(-1), r(a), c(b), tr(0), tc(0), ro(x) {}
	Shx(const Shx &p) : id(p.id), trid(p.trid), r(p.r), c(p.c), tr(p.tr), tc(p.tc), ro(p.ro) {}

	Shx &operator=(const Shx &p) {
		id = p.id;
		trid = p.trid;
		r = p.r;
		c = p.c;
		tr = p.tr;
		tc = p.tc;
		ro = p.ro;
		return *this;
	}
};

int triangulateImpl(std::vector<Shx> &points, std::vector<Triad> &triads);

template<class VERTEX_TYPE>
int triangulate(VERTEX_TYPE* points, unsigned nPoints, std::vector<Triangle> &triangles) {
	std::vector<Shx> vshx;
	vshx.reserve(nPoints);
	for (unsigned i=0; i<nPoints; i++) {
		vshx.emplace_back(Shx{nth_elem(points[i], 0), nth_elem(points[i], 1)});
		vshx.back().id = i;
	}
	std::vector<Triad> triads;
	int ret = triangulateImpl(vshx, triads);
	if (ret < 0)
		return ret;
	// populate triangles vec
	triangles.clear();
	triangles.reserve(triads.size());
	for (auto &t : triads)
		triangles.emplace_back(Triangle{t.a, t.b, t.c, t.ab, t.ac, t.bc});
	return ret;
}

#endif // __TRIANGULATION_H__
