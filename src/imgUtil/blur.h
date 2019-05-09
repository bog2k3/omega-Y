#pragma once

namespace imgUtil {

// blurs an image over a given radius
// <pixelType> must support the following operators: * (float), / (float), + (pixelType)
template <class pixelType>
void blur(const pixelType* in, int rows, int cols, float radius, pixelType* out);

} // namespace

#include "blur.private.h"
