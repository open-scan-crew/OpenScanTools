#ifndef DUPLICATION_TYPES_H
#define DUPLICATION_TYPES_H

#include <glm/glm.hpp>

enum class DuplicationMode
{
    Click,
    SizeStep,
    Offset
};

struct DuplicationSettings
{
    glm::dvec3 offset;
    DuplicationMode type;
    glm::ivec3 step;
    bool	isLocal;
};

#endif