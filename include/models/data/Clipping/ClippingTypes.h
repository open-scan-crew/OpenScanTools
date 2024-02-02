#ifndef CLIPPING_TYPES_H
#define CLIPPING_TYPES_H

#include <cstdint>

enum class ClippingMode
{
    showInterior,
    showExterior,
    colorInterior,
    Max_Enum

};

// Add ClippingShape ?

typedef uint16_t ClippingGpuId;

#endif
