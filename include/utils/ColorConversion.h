#ifndef COLOR_CONVERSION_H
#define COLOR_CONVERSION_H

#include "glm/glm.hpp"

namespace utils::color
{
    float hue2rgb(float f1, float f2, float hue);

    glm::vec3 hsl2rgb(glm::vec3 hsl);
    glm::vec3 rgb2hsl(glm::vec3 rgb);

    glm::vec3 hsv2rgb(glm::vec3 hsv);
    glm::vec3 rgb2hsv(glm::vec3 rgb);
}

#endif