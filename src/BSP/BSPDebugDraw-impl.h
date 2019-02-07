#ifndef __BSP_DEBUG_DRAW_IMPL_H__
#define __BSP_DEBUG_DRAW_IMPL_H__

#include "BSP.h"

#include <boglfw/renderOpenGL/Shape3D.h>

template<class ObjectType, bool dynamic>
void BSPDebugDraw::drawNode(BSPNode<ObjectType, dynamic> const& node) {
	if (node.positive_) {
		drawNode<ObjectType, dynamic>(*node.positive_);
		drawNode<ObjectType, dynamic>(*node.negative_);
		return;
		// this was not a leaf node
	}
	// this is a leaf node, draw its AABB
	Shape3D::get()->drawAABB(node.aabb_, glm::vec3(0.f, 1.f, 0.f));
}

template<class ObjectType, bool dynamic>
void BSPDebugDraw::draw(BSPTree<ObjectType, dynamic> const& tree) {
	drawNode<ObjectType, dynamic>(*tree.root_);
}

#endif // __BSP_DEBUG_DRAW_IMPL_H__
