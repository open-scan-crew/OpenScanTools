#include "controller/functionSystem/ContextAlignView3P.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "gui/GuiData/GuiDataMessages.h"

#include "gui/texts/AlignViewTexts.hpp"
#include "gui/texts/RayTracingTexts.hpp"

#include "models/graph/CameraNode.h"

#include "utils/math/trigo.h"

ContextAlignView3P::ContextAlignView3P(const ContextId& id)
    : AContextAlignView(id)
{
    // Set the description of the points needed for the processing
    m_usages.push_back({ true, {ElementType::Tag, ElementType::Point}, TEXT_RAYTRACING_DEFAULT_1 });
    m_usages.push_back({ true, {ElementType::Tag, ElementType::Point}, TEXT_RAYTRACING_DEFAULT_2 });
    m_usages.push_back({ true, {ElementType::Tag, ElementType::Point}, TEXT_RAYTRACING_DEFAULT_3 });
}

ContextAlignView3P::~ContextAlignView3P()
{}

ContextState ContextAlignView3P::launch(Controller& controller)
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

    glm::dvec3 A(m_clickResults[1].position - m_clickResults[0].position);
    glm::dvec3 B(m_clickResults[2].position - m_clickResults[0].position);
    glm::dvec3 C;
    if (rCam->getProjectionMode() == ProjectionMode::Perspective)
        C = m_clickResults[0].position - rCam->getCenter();
    else
        C = rCam->getTransformation() * glm::dvec4(0.0, 0.0, 1.0, 0.0);
    glm::dvec3 N(glm::normalize(glm::cross(A, B))); // TODO check null vector
    glm::dvec3 M = (m_clickResults[0].position + m_clickResults[1].position + m_clickResults[2].position) / 3.0;

	if (glm::dot(N, C) < 0)
	{
		N = -N;
	}
    double theta(atan2(-N.x, N.y));
    double phi(-acos(N.z));
    double dist = glm::length(M - rCam->getCenter());
    glm::dvec3 camNewPos = M - N * dist;

    if (!std::isnan(theta) && !std::isnan(phi))
    {
        controller.updateInfo(new GuiDataRenderCameraMoveTo(glm::dvec4(camNewPos, 1.0), m_cameraNode));
        controller.updateInfo(new GuiDataRenderRotateCamera(theta, phi, false, m_cameraNode));
        controller.updateInfo(new GuiDataTmpMessage(""));
    }
    else
    {
        m_clickResults.clear();
        return waitForNextPoint(controller, TEXT_ALIGN_VIEW_FAILED);
    }

	return ARayTracingContext::validate(controller);
}

ContextType ContextAlignView3P::getType() const
{
	return ContextType::alignView3P;
}
