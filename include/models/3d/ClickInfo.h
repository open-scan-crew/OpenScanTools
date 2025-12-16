#ifndef CLICK_INFO_H
#define CLICK_INFO_H

#include "tls_def.h"
#include "utils/safe_ptr.h"


#include <glm/glm.hpp>
#include <memory>

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
    SafePtr<AGraphNode> hover;
    std::shared_ptr<MeshBuffer> mesh;
    glm::dmat4 meshTransfo;
    glm::dvec3 picking; // viewport picking from ClickMessage
    glm::dvec3 ray;
    glm::dvec3 rayOrigin;
    tls::ScanGuid panoramic;
    SafePtr<CameraNode> viewport;
    bool useObjectCenter = false;

    ClickInfo(uint32_t width = 0,
              uint32_t height = 0,
              bool ctrl = false,
              double fov = 0.0,
              double heightAt1m = 0.0,
              SafePtr<AGraphNode> hover = SafePtr<AGraphNode>(),
              std::shared_ptr<MeshBuffer> mesh = nullptr,
              glm::dmat4 meshTransfo = glm::dmat4(),
              glm::dvec3 picking = glm::dvec3(),
              glm::dvec3 ray = glm::dvec3(),
              glm::dvec3 rayOrigin = glm::dvec3(),
              tls::ScanGuid panoramic = tls::ScanGuid(),
              SafePtr<CameraNode> viewport = SafePtr<CameraNode>(),
              bool useObjectCenter = false)
        : width(width)
        , height(height)
        , ctrl(ctrl)
        , fov(fov)
        , heightAt1m(heightAt1m)
        , hover(hover)
        , mesh(mesh)
        , meshTransfo(meshTransfo)
        , picking(picking)
        , ray(ray)
        , rayOrigin(rayOrigin)
        , panoramic(panoramic)
        , viewport(viewport)
        , useObjectCenter(useObjectCenter)
    {
    }
};

#endif
