#ifndef __BSP_IMPL_H__
#define __BSP_IMPL_H__

#include "BSP.h"

// ----------------------------------------------------------------------------------------------------------//
// ----------------------------- Template implementation follows --------------------------------------------//
// ----------------------------------------------------------------------------------------------------------//

template<class ObjectType, bool dynamic>
BSPTree<ObjectType, dynamic>
::BSPTree(BSPConfig const& config, AABBGeneratorInterface<object_type>* pAABBGenerator, std::vector<object_type> &&objects) {
	AABB rootAABB;
	if (config.targetVolume.isEmpty()) {
		// compute from objects
		for (auto &obj : objects)
			rootAABB.expand(pAABBGenerator->getAABB(obj));
	} else
		rootAABB = config.targetVolume;

	root_ = new node_type(pAABBGenerator, nullptr, rootAABB, std::move(objects));
	root_->split(config);
}

template<class ObjectType, bool dynamic>
typename BSPTree<ObjectType, dynamic>::node_type*
BSPTree<ObjectType, dynamic>::getNodeAtPoint(glm::vec3 const& p) const {
	node_type* n = root_;
	while (n->positive_) {
		float q = n->splitPlane_.x * p.x + n->splitPlane_.y * p.y + n->splitPlane_.z * p.z + n->splitPlane_.w;
		if (q >= 0)
			n = n->positive_;
		else
			n = n->negative_;
	}
	return n;
}

template<class ObjectType, bool dynamic>
BSPNode<ObjectType, dynamic>
::BSPNode(AABBGeneratorInterface<ObjectType>* aabbGenerator, node_type* parent, AABB aabb, std::vector<ObjectType> &&objects)
	: aabbGenerator_(aabbGenerator)
	, aabb_(aabb)
	, objects_(std::move(objects))
	, parent_(parent)
{
}

template<class ObjectType, bool dynamic>
void
BSPNode<ObjectType, dynamic>::split(BSPConfig const& config) {
	if (objects_.size() <= config.minObjects)
		return; // too few objects to split
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
		if (config.maxDepth[splitAxis] != 0 && (depth_[splitAxis] + 1) > config.maxDepth[splitAxis])
			continue; // we would exceed maxDepth in the requested direction so we don't split on this axis
		// compute the new AABBs of the would-be child nodes
		float splitCoord = (aabb_.vMin[splitAxis] + aabb_.vMax[splitAxis]) * 0.5f;
		AABB aabbNegative(aabb_);
		aabbNegative.vMax[splitAxis] = splitCoord;
		AABB aabbPositive(aabb_);
		aabbPositive.vMin[splitAxis] = splitCoord;
		// check if the AABBs of the children are not smaller than the minCellSize
		if (aabbNegative.size()[splitAxis] < config.minCellSize[splitAxis]
			|| aabbPositive.size()[splitAxis] < config.minCellSize[splitAxis])
			continue; // too small would-be cells, we don't split on this axis

		// all right, now we split
		splitPlane_[splitAxis] = 1.f;
		splitPlane_.w = -splitCoord;
		std::vector<ObjectType> objectsNeg;
		std::vector<ObjectType> objectsPos;
		for (unsigned k=0; k<objects_.size(); k++) {
			switch(aabbGenerator_->getAABB(objects_[k]).qualifyPlane(splitPlane_)) {
				case 0:
					objectsNeg.push_back(objects_[k]);
					objectsPos.push_back(objects_[k]);
					break;
				case 1:
					objectsPos.push_back(objects_[k]);
					break;
				case -1:
					objectsNeg.push_back(objects_[k]);
					break;
				default:
					assertDbg("wtf");
					break;
			}
		}
		negative_ = new BSPNode<ObjectType, dynamic>(aabbGenerator_, this, aabbNegative, std::move(objectsNeg));
		negative_->depth_ = depth_;
		negative_->depth_[splitAxis]++;
		negative_->split(config);
		positive_ = new BSPNode<ObjectType, dynamic>(aabbGenerator_, this, aabbPositive, std::move(objectsPos));
		positive_->depth_ = depth_;
		positive_->depth_[splitAxis]++;
		positive_->split(config);

		break; // we're done splitting, we don't care about the remaining potential axes
	}
}

#endif // __BSP_IMPL_H__
