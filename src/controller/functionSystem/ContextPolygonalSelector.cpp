#include "controller/functionSystem/ContextPolygonalSelector.h"

#include "controller/Controller.h"
#include "controller/controls/ControlFunction.h"
#include "controller/messages/FullClickMessage.h"
#include "controller/messages/IMessage.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/texts/ContextTexts.hpp"
#include "models/graph/CameraNode.h"

#include <glm/gtx/norm.hpp>
#include <algorithm>
#include <cmath>

namespace
{
    PolygonalSelectorSettings s_polygonalSelectorRuntimeSettings;
    PolygonalSelectorCameraSnapshot s_lastSnapshot;
}

ContextPolygonalSelector::ContextPolygonalSelector(const ContextId& id)
    : AContext(id)
{}

ContextPolygonalSelector::~ContextPolygonalSelector()
{}

ContextState ContextPolygonalSelector::start(Controller& controller)
{
    m_settings = s_polygonalSelectorRuntimeSettings;
    m_currentVertices.clear();
    controller.updateInfo(new GuiDataRenderPolygonalSelector(m_settings, SafePtr<CameraNode>(), m_currentVertices, false));
    controller.updateInfo(new GuiDataTmpMessage(TEXT_POINT_MEASURE_START));
    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextPolygonalSelector::feedMessage(IMessage* message, Controller& controller)
{
    if (message->getType() == IMessage::MessageType::FULL_CLICK)
    {
        auto* clickMsg = static_cast<FullClickMessage*>(message);
        const ClickInfo& click = clickMsg->m_clickInfo;
        glm::vec2 normalized(0.0f, 0.0f);

        ReadPtr<CameraNode> rCam = click.viewport.cget();
        if (rCam && click.width > 0 && click.height > 0)
        {
            glm::dvec3 rayDir = glm::length2(click.ray) > 0.0 ? glm::normalize(click.ray) : glm::dvec3(0.0, 0.0, -1.0);
            glm::dvec3 markerPoint = click.rayOrigin + rayDir;
            glm::dvec3 screen = rCam->getScreenProjection(markerPoint, glm::ivec2(click.width, click.height));

            normalized.x = static_cast<float>(screen.x / static_cast<double>(click.width));
            normalized.y = static_cast<float>(screen.y / static_cast<double>(click.height));
            normalized.x = std::clamp(normalized.x, 0.0f, 1.0f);
            normalized.y = std::clamp(normalized.y, 0.0f, 1.0f);
        }

        if (!std::isfinite(normalized.x) || !std::isfinite(normalized.y))
            return (m_state = ContextState::waiting_for_input);

        m_currentVertices.emplace_back(normalized);

        if (rCam)
        {
            s_lastSnapshot.view = rCam->getViewMatrix();
            s_lastSnapshot.proj = rCam->getProjMatrix();
            s_lastSnapshot.viewportWidth = click.width;
            s_lastSnapshot.viewportHeight = click.height;
            s_lastSnapshot.perspective = (rCam->getProjectionMode() == ProjectionMode::Perspective);
        }

        controller.updateInfo(new GuiDataRenderPolygonalSelector(m_settings, SafePtr<CameraNode>(), m_currentVertices, false));
        controller.updateInfo(new GuiDataTmpMessage(QString("Polygonal selector: %1 vertex/vertices").arg(m_currentVertices.size())));
    }

    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextPolygonalSelector::launch(Controller& controller)
{
    (void)controller;
    return (m_state = ContextState::waiting_for_input);
}

ContextState ContextPolygonalSelector::abort(Controller& controller)
{
    m_currentVertices.clear();
    controller.updateInfo(new GuiDataRenderPolygonalSelector(m_settings, SafePtr<CameraNode>(), m_currentVertices, false));
    return AContext::abort(controller);
}

ContextState ContextPolygonalSelector::validate(Controller& controller)
{
    if (m_currentVertices.size() >= 3)
    {
        PolygonalSelectorPolygon polygon;
        polygon.normalizedVertices = m_currentVertices;
        polygon.camera = s_lastSnapshot;

        controller.updateInfo(new GuiDataRenderPolygonalSelector(m_settings, SafePtr<CameraNode>(), m_currentVertices, true));

        m_settings.polygons.push_back(polygon);
        m_settings.enabled = true;
        m_settings.active = true;
        s_polygonalSelectorRuntimeSettings = m_settings;
    }

    m_currentVertices.clear();
    controller.updateInfo(new GuiDataRenderPolygonalSelector(m_settings, SafePtr<CameraNode>(), m_currentVertices, false));
    return AContext::validate(controller);
}

bool ContextPolygonalSelector::canAutoRelaunch() const
{
    return true;
}

ContextType ContextPolygonalSelector::getType() const
{
    return ContextType::polygonalSelector;
}
