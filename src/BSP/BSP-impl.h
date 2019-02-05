#ifndef __BSP_IMPL_H__
#define __BSP_IMPL_H__

#include "BSP.h"

// ----------------------------------------------------------------------------------------------------------//
// ----------------------------- Template implementation follows --------------------------------------------//
// ----------------------------------------------------------------------------------------------------------//

template<class ValueType, class aabbGenerator, bool dynamic>
BSPTree<ValueType, aabbGenerator, dynamic>
::BSPTree(glm::ivec3 maxLayers, glm::vec3 minCellSize, ValueType* pValues, unsigned nValues) {

}

#endif // __BSP_IMPL_H__
