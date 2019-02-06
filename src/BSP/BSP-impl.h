#ifndef __BSP_IMPL_H__
#define __BSP_IMPL_H__

#include "BSP.h"

// ----------------------------------------------------------------------------------------------------------//
// ----------------------------- Template implementation follows --------------------------------------------//
// ----------------------------------------------------------------------------------------------------------//

template<class ValueType, bool dynamic>
BSPTree<ValueType, dynamic>
::BSPTree(AABBGeneratorInterface<ValueType> *pAABBGenerator, glm::ivec3 const& maxLayers,
			glm::vec3 const& minCellSize, ValueType* pValues, unsigned nValues) {
	const float inf = 1.e10f;
	AABB rootAABB(glm::vec3(-inf, -inf, -inf), glm::vec3(inf, inf, inf));
	std::vector<ValueType> values;
	values.reserve(nValues);
	for (unsigned i=0; i<nValues; i++)
		values.push_back(pValues[i]);
	root_ = new node_type(nullptr, rootAABB, std::move(values));
	int splitDir = 1;	// X
	if (max(maxLayers.y, maxLayers.z) > maxLayers.x) {
		if (maxLayers.y > maxLayers.z)
			splitDir = 2; // Y
		else
			splitDir = 3; // Z
	}
	root_->split(splitDir, maxLayers, minCellSize);
}

template<class ValueType, bool dynamic>
BSPNode<ValueType, dynamic>
::BSPNode(node_type* parent, AABB aabb, std::vector<ValueType> &&values)
	: parent_(parent)
	, aabb_(aabb)
	, values_(std::move(values))
{
}

template<class ValueType, bool dynamic>
void
BSPNode<ValueType, dynamic>
::split(int splitDirection, glm::ivec3 const& maxLayers, glm::vec3 const& minCellSize) {
	// we must decide if the split is actually going to take place according to the rules
}

#endif // __BSP_IMPL_H__
