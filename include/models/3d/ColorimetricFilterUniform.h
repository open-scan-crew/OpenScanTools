#ifndef COLORIMETRIC_FILTER_UNIFORM_H
#define COLORIMETRIC_FILTER_UNIFORM_H

#include <array>

#include <glm/glm.hpp>

struct ColorimetricFilterUniform
{
    glm::vec4 settings = glm::vec4(0.0f);
    std::array<glm::vec4, 4> colors = { glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f) };
};

#endif
