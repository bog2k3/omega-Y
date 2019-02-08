#ifndef __BSP_DEBUG_DRAW_H__
#define __BSP_DEBUG_DRAW_H__

#include "BSP.h"

class BSPDebugDraw {
public:
	template<class ObjectType>
	static void draw(BSPTree<ObjectType> const& tree);

	template<class ObjectType>
	static void drawNode(BSPNode<ObjectType> const& node);
};

#include "BSPDebugDraw-impl.h"

#endif // __BSP_DEBUG_DRAW_H__
