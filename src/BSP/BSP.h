#ifndef __BSP_H__
#define __BSP_H__

#include <boglfw/math/aabb.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vector>
#include <map>

// Scroll down to BSPTree main class for the main interface.

// ----------------------------- Helper declarations here --------------------------------------------//

template<class ValueType, class aabbGenerator, bool dynamic>
class BSPTree;

template<class ValueType, class aabbGenerator, bool dynamic>
class BSPNode {
public:
	using value_type = ValueType;

	const std::vector<ValueType>& values() { return values_; }

protected:
	friend class BSPTree<ValueType, aabbGenerator, dynamic>;

	AABB aabb_;
	std::vector<ValueType> values_;
	BSPNode<ValueType, aabbGenerator, dynamic> *negative_ = nullptr;
	BSPNode<ValueType, aabbGenerator, dynamic> *positive_ = nullptr;
	BSPNode<ValueType, aabbGenerator, dynamic> *parent_ = nullptr;
	int splitDirection_ = 0;	// 1=X, 2=Y, 3=Z
	glm::vec4 splitPlane_ {0.f};	//x=a, y=b, z=c, w=d
};

// ----------------------------- AABBGeneratorInterface --------------------------------------------//

template<class ValueType>
class AABBGeneratorInterface {
public:
	// return the axis-aligned bounding box for a given value
	AABB getAABB(ValueType const& v) = 0;
};

// -------------------------------------------------------------------------------------------------//
// ----------------------------- BSPTree main class ------------------------------------------------//
// -------------------------------------------------------------------------------------------------//

// BSPTree that contains elements of type ValueType.
// You must provide an implementation of AABBGeneratorInterface that returns axis-aligned bounding boxes for your elements.
// [dynamic] template parameter enables the ability to dynamically add and remove elements from the BSPTree after it has been created.
// Only use [dynamic] when you really need it, because it implies a performance cost.
template<class ValueType, class aabbGenerator = AABBGeneratorInterface<ValueType>, bool dynamic = false>
class BSPTree {
public:
	using value_type = ValueType;
	using node_type = BSPNode<ValueType, aabbGenerator, dynamic>;

	// [maxLayers] specifies the maximum number of layers in each spatial direction
	// a value of 0 for an axis means unlimitted layers in that direction.
	// [minCellSize] specifies the minimum cell size for each spatial direction
	// a value of 0.f for an axis means no limit on the cell size on that axis
	BSPTree(glm::ivec3 maxLayers, glm::vec3 minCellSize, ValueType* pValues, unsigned nValues);

	// Returns a list of leaf nodes that are intersected by or contained within a given AABB
	// If one or more vertices of the AABB lie strictly on the boundary of some nodes, those nodes are not returned.
	std::vector<node_type*> getNodesAABB(AABB const& aabb_) const;

	// Returns the leaf node that contains the point p. If the point is on the boundary between two nodes,
	// the "positive" node (on the positive side of the separation plane) is chosen by convention.
	// Separation planes are always oriented with the positive side in the positive direction of the corresponding world axis.
	node_type* getNodeAtPoint(glm::vec3 const& p) const;

private:
	node_type *root_ = nullptr;
	std::map<ValueType, node_type*> valueNodes_; // maps values back to the nodes they belong to (only for dynamic usage)
};

#include "BSP-impl.h"

#endif // __BSP_H__
