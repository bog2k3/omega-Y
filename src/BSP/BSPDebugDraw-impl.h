#ifndef __BSP_DEBUG_DRAW_IMPL_H__
#define __BSP_DEBUG_DRAW_IMPL_H__

#include "BSPDebugDraw.h"

#include <boglfw/renderOpenGL/Shape3D.h>

template<class ObjectType>
void BSPDebugDraw::drawNode(BSPNode<ObjectType> const& node) {
	if (node.positive_) {
		drawNode<ObjectType>(*node.positive_);
		drawNode<ObjectType>(*node.negative_);
		return;
		// this was not a leaf node
	}
	// this is a leaf node, draw its AABB
	Shape3D::get()->drawAABB(node.aabb_, glm::vec3(0.f, 1.f, 0.f));
}

template<class ObjectType>
void BSPDebugDraw::draw(BSPTree<ObjectType> const& tree) {
	drawNode<ObjectType>(*tree.root_);
}

#endif // __BSP_DEBUG_DRAW_IMPL_H__
