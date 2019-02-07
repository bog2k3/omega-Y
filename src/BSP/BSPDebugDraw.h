#ifndef __BSP_DEBUG_DRAW_H__
#define __BSP_DEBUG_DRAW_H__

#include "BSP.h"

class BSPDebugDraw {
public:
	template<class ObjectType, bool dynamic>
	static void draw(BSPTree<ObjectType, dynamic> const& tree);

	template<class ObjectType, bool dynamic>
	static void drawNode(BSPNode<ObjectType, dynamic> const& node);
};

#include "BSPDebugDraw-impl.h"

#endif // __BSP_DEBUG_DRAW_H__
