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
	root_ = new node_type(pAABBGenerator, (node_type*)nullptr, rootAABB, std::move(values));
	root_->split(maxLayers, minCellSize);
}

template<class ValueType, bool dynamic>
BSPNode<ValueType, dynamic>
::BSPNode(AABBGeneratorInterface<ValueType>* aabbGenerator, node_type* parent, AABB aabb, std::vector<ValueType> &&values)
	: aabbGenerator_(aabbGenerator)
	, parent_(parent)
	, aabb_(aabb)
	, values_(std::move(values))
{
}

template<class ValueType, bool dynamic>
void
BSPNode<ValueType, dynamic>
::split(glm::ivec3 const& maxLayers, glm::vec3 const& minCellSize) {
	// determine the potential split axes:
	glm::vec3 crtCellSize = aabb_.size();
	int splitAxes[3] {0, 1, 2};
	// sort the axes in the order of preference -> a larger box size in one direction is preferred.
	for (int i=0; i<2; i++)
		for(int j=i+1; j<3; j++)
			if (crtCellSize[splitAxes[i]] < crtCellSize[splitAxes[j]]) {
				xchg(splitAxes[i], splitAxes[j]);
			}
	for (int i=0; i<3; i++) {
		int splitAxis = splitAxes[i];
		// we must decide if the split on the current axis is actually possible according to the rules
		glm::ivec3 addedDepth {0};
		addedDepth[splitAxis] = 1;
		if ((depth_ + addedDepth)[splitAxis] > maxLayers[splitAxis])
			continue; // we would exceed maxLayers in the requested direction so we don't split on this axis
		// compute the new AABBs of the would-be child nodes
		float splitCoord = (aabb_.vMin[splitAxis] + aabb_.vMax[splitAxis]) * 0.5f;
		AABB aabbNegative(aabb_);
		aabbNegative.vMax[splitAxis] = splitCoord;
		AABB aabbPositive(aabb_);
		aabbPositive.vMin[splitAxis] = splitCoord;
		// check if the AABBs of the children are not smaller than the minCellSize
		if (aabbNegative.size()[splitAxis] < minCellSize[splitAxis]
			|| aabbPositive.size()[splitAxis] < minCellSize[splitAxis])
			continue; // too small would-be cells, we don't split on this axis

		// all right, now we split
		splitPlane_[splitAxis] = 1.f;
		splitPlane_.w = -splitCoord;
		std::vector<ValueType> valuesNeg;
		std::vector<ValueType> valuesPos;
		for (unsigned k=0; k<values_.size(); k++) {
			switch(aabbGenerator_->getAABB(values_[k]).qualifyPlane(splitPlane_)) {
				case 0:
					valuesNeg.push_back(values_[k]);
					valuesPos.push_back(values_[k]);
					break;
				case 1:
					valuesPos.push_back(values_[k]);
					break;
				case -1:
					valuesNeg.push_back(values_[k]);
					break;
				default:
					assertDbg("wtf");
					break;
			}
		}
		negative_ = new BSPNode<ValueType, dynamic>(aabbGenerator_, this, aabbNegative, std::move(valuesNeg));
		positive_ = new BSPNode<ValueType, dynamic>(aabbGenerator_, this, aabbPositive, std::move(valuesPos));
		negative_->split(maxLayers, minCellSize);
		positive_->split(maxLayers, minCellSize);

		break; // we're done splitting, we don't care about the remaining potential axes
	}
}

#endif // __BSP_IMPL_H__
