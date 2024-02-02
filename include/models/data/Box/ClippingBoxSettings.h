#ifndef CLIPPING_BOX_SETTINGS_H
#define CLIPPING_BOX_SETTINGS_H

#include <glm/glm.hpp>

enum class ClippingBoxOffset
{
    CenterOnPoint,
    BottomFace,
    Topface,
};

struct ClippingBoxSettings
{
    double angleZ;
    glm::vec3 size;
    ClippingBoxOffset offset;
};

#endif