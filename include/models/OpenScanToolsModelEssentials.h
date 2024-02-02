#ifndef OST_MODEL_ESSENTIALS_H
#define OST_MODEL_ESSENTIALS_H

#include "glm/glm.hpp"
#include "utils/safe_ptr.h"

enum class ObjectStatusFilter { ALL, SELECTED, VISIBLE, NONE };

typedef glm::dvec3 Pos3D;


struct Rect2D {
    glm::ivec2 c0; // first corner
    glm::ivec2 c1; // opposite corner
};

#define INVALID_PICKING_ID		0xFFFFFFFF
#define RESERVED_DATA_ID_MASK   0x80000000
#define VALUE_DATA_ID_MASK		0x7FFFFFFF

#endif // !OpenScanTools_MODEL_ESSENTIALS_H