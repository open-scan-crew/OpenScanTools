#ifndef UNIFORM_COLOR_FILTER_H
#define UNIFORM_COLOR_FILTER_H

#include <glm/glm.hpp>

struct UniformColorFilter
{
    glm::vec4 params;
    glm::vec4 colorA;
    glm::vec4 colorB;
    glm::vec4 colorC;
};

#endif // UNIFORM_COLOR_FILTER_H
