#ifndef MESH_DRAW_DATA_H
#define MESH_DRAW_DATA_H

#include <cstdint>
#include <glm/glm.hpp>
#include <memory>
#include "models/3d/MeshBuffer.h"

struct MeshDrawData
{
    uint32_t graphicId;
    glm::vec3 color;
    glm::dmat4 transfo;
    bool isHovered;
    bool isSelected;
    std::shared_ptr<MeshBuffer> meshBuffer = std::shared_ptr<MeshBuffer>();
};

#endif