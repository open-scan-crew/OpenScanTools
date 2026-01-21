#ifndef POINT_CLOUD_DRAW_DATA_H
#define POINT_CLOUD_DRAW_DATA_H

#include "tls_def.h"
#include "utils/Color32.hpp"
#include "vulkan/VkUniform.h"
#include <glm/glm.hpp>
#include <string>

struct PointCloudDrawData
{
    glm::dmat4 transfo;
    tls::ScanGuid scanGuid;
    Color32 color;
    VkUniformOffset uniform;
    bool clippable;
    std::wstring phase;

    bool isObject;
    // #ClippingAssembly ?
}; // size (128 + 16 + 4 + 4) = 152


#endif
