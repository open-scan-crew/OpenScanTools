#ifndef PCE_GRAPHICS_H
#define PCE_GRAPHICS_H

//-----------------------------
//**** Graphical functions ****
//-----------------------------

#include "tls_def.h"
#include "models/data/Clipping/ClippingTypes.h"
#include "vulkan/VkUniform.h"
#include "vulkan/vulkan_core.h"

#include <glm/glm.hpp>
#include <vector>


struct TlCellDrawInfo
{
    VkBuffer buffer;
    uint32_t cellIndex;
    uint32_t vertexCount;
    uint32_t m_iOffset = 0;
    uint32_t m_rgbOffset = 0;
};

struct TlCellDrawInfo_multiCB
{
    VkBuffer buffer;
    uint32_t cellIndex;
    uint32_t vertexCount;
    uint32_t m_iOffset = 0;
    uint32_t m_rgbOffset = 0;
    std::vector<ClippingGpuId> clippingGpuIds;
};

struct TlScanDrawInfo
{
    VkBuffer instanceBuffer;
    VkUniformOffset modelUni;
    VkUniformOffset colorFilterUni;
    tls::PointFormat format;
    std::vector<TlCellDrawInfo> cellDrawInfo;
    std::vector<TlCellDrawInfo_multiCB> cellDrawInfoCB;
    glm::vec3 color;
};


// Scan vision
struct TlProjectionInfo
{
    glm::dmat4 modelMat = glm::dmat4();
    glm::dmat4 frustumMat = glm::dmat4();
    uint32_t width = 0;
    uint32_t height = 0;
    float octreePointSize = 1.f;
    float decimationRatio = 1.f;
    float deltaFilling = 0.f;
};

#endif
