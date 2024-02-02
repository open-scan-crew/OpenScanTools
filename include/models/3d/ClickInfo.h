#ifndef CLICK_INFO_H
#define CLICK_INFO_H

#include "models/pointCloud/TLS.h"
#include "models/OpenScanToolsModelEssentials.h"

#include <glm/glm.hpp>

class MeshBuffer;

class AGraphNode;
class CameraNode;

struct ClickInfo
{
    uint32_t width;
    uint32_t height;
    bool ctrl;
    double fov;
    double heightAt1m;
    SafePtr<AGraphNode> hover; // FIXME - Le AGraphNode ne peut pas être "hover". C’est une propriété du AObjectNode.
    std::shared_ptr<MeshBuffer> mesh;
    glm::dmat4 meshTransfo;
    glm::dvec3 picking; // viewport picking from ClickMessage
    glm::dvec3 ray;
    glm::dvec3 rayOrigin;
    tls::ScanGuid panoramic;
    SafePtr<CameraNode> viewport;
};

#endif