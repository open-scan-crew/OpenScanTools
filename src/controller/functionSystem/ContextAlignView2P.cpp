#include "controller/functionSystem/ContextAlignView2P.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "controller/Controller.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/AlignViewTexts.hpp"
#include "gui/texts/RayTracingTexts.hpp"
#include "models/graph/CameraNode.h"
#include "utils/math/basic_define.h"

ContextAlignView2P::ContextAlignView2P(const ContextId& id)
    : AContextAlignView(id)
{
    // Set the description of the points needed for the processing
    m_usages.push_back({ true, {ElementType::Tag, ElementType::Point}, TEXT_RAYTRACING_DEFAULT_1 });
    m_usages.push_back({ true, {ElementType::Tag, ElementType::Point}, TEXT_RAYTRACING_DEFAULT_2 });
}

ContextAlignView2P::~ContextAlignView2P()
{}

ContextState ContextAlignView2P::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
    if (pointMissing())
        return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

    ReadPtr<CameraNode> rCam = m_cameraNode.cget();
    if (!rCam)
    {
        assert(false);
        return ARayTracingContext::abort(controller);
    }

    glm::dvec3 A(m_clickResults[1].position.x - m_clickResults[0].position.x, m_clickResults[1].position.y - m_clickResults[0].position.y, 0.0);
    A = glm::normalize(A);
    glm::dvec3 M((m_clickResults[1].position + m_clickResults[0].position) / 2.0);
    glm::dvec3 Z(0.0, 0.0, 1.0);
    glm::dvec3 N(glm::cross(A, Z));
    glm::dvec3 C(1.0);
    if (rCam->getProjectionMode() == ProjectionMode::Perspective)
        C = M - rCam->getCenter();
    else
        C = rCam->getTransformation() * glm::dvec4(0.0, 0.0, 1.0, 0.0);

    if (glm::dot(C, N) < 0)
    {
        N = -N;
    }
    // Compute the camera position : at the same distance to M but aligned with N
    double dist = glm::length(M - rCam->getCenter());
    glm::dvec3 camNewPos = M - N * dist;
    double angle = atan2(-N.x, N.y);

    if (!std::isnan(angle))
    {
        controller.updateInfo(new GuiDataRenderCameraMoveTo(glm::dvec4(camNewPos, 1.0), m_cameraNode));
        controller.updateInfo(new GuiDataRenderRotateCamera(angle, -M_PI2, false, m_cameraNode));
        controller.updateInfo(new GuiDataTmpMessage(""));
    }
    else
    {
        m_clickResults.clear();
        return waitForNextPoint(controller, TEXT_ALIGN_VIEW_FAILED);
    }

    return ARayTracingContext::validate(controller);
}

ContextType ContextAlignView2P::getType() const
{
    return ContextType::alignView2P;
}
