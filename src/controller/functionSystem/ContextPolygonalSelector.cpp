#include "controller/functionSystem/ContextPolygonalSelector.h"

#include "controller/Controller.h"
#include "controller/controls/ControlFunction.h"
#include "controller/messages/FullClickMessage.h"
#include "controller/messages/IMessage.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "gui/texts/ContextTexts.hpp"
#include "models/graph/CameraNode.h"
#include "models/graph/GraphManager.h"
#include "pointCloudEngine/RenderingLimits.h"

#include <glm/gtx/norm.hpp>
#include <algorithm>
#include <cmath>

namespace
{
uint32_t getPolygonSuffix(const std::string& name)
{
    if (name.rfind("polygon_", 0) != 0)
        return 0;

    bool ok = false;
    int suffix = QString::fromStdString(name.substr(8)).toInt(&ok);
    return (ok && suffix > 0) ? static_cast<uint32_t>(suffix) : 0;
}

uint32_t computeNextPolygonId(const PolygonalSelectorSettings& settings)
{
    uint32_t maxSuffix = 0;
    for (const PolygonalSelectorPolygon& polygon : settings.polygons)
        maxSuffix = std::max<uint32_t>(maxSuffix, getPolygonSuffix(polygon.name));

    return std::max<uint32_t>(std::max<uint32_t>(settings.nextPolygonId, maxSuffix + 1), 1u);
}

void captureSnapshotClipping(const ClippingAssembly& assembly, PolygonalSelectorPolygon& polygon)
{
    polygon.snapshotUnion.clear();
    polygon.snapshotIntersection.clear();

    auto convert = [](const std::shared_ptr<IClippingGeometry>& geom) -> PolygonalSelectorPolygon::SnapshotClip
    {
        PolygonalSelectorPolygon::SnapshotClip snapshot;
        snapshot.shape = static_cast<int32_t>(geom->getShape());
        snapshot.mode = static_cast<int32_t>(geom->mode);
        snapshot.matRTInv = geom->matRT_inv_store;
        snapshot.params = geom->params;
        return snapshot;
    };

    for (const std::shared_ptr<IClippingGeometry>& geom : assembly.clippingUnion)
    {
        if (polygon.snapshotUnion.size() >= MAX_POLYGONAL_SELECTOR_SNAPSHOT_CLIPS)
            break;
        polygon.snapshotUnion.push_back(convert(geom));
    }

    for (const std::shared_ptr<IClippingGeometry>& geom : assembly.clippingIntersection)
    {
        if (polygon.snapshotUnion.size() + polygon.snapshotIntersection.size() >= MAX_POLYGONAL_SELECTOR_SNAPSHOT_CLIPS)
            break;
        polygon.snapshotIntersection.push_back(convert(geom));
    }
}
}

ContextPolygonalSelector::ContextPolygonalSelector(const ContextId& id)
    : AContext(id)
{}

ContextPolygonalSelector::~ContextPolygonalSelector()
{}

ContextState ContextPolygonalSelector::start(Controller& controller)
{
    m_currentVertices.clear();

    SafePtr<CameraNode> cam = controller.getGraphManager().getCameraNode();
    ReadPtr<CameraNode> rCam = cam.cget();
    if (rCam)
    {
        m_settings = rCam->getDisplayParameters().m_polygonalSelector;
        m_settings.appliedPolygonCount = std::min<uint32_t>(m_settings.appliedPolygonCount, static_cast<uint32_t>(m_settings.polygons.size()));
        m_settings.nextPolygonId = computeNextPolygonId(m_settings);
        if (m_settings.appliedPolygonCount == m_settings.polygons.size())
            m_settings.pendingApply = false;
    }
    else
    {
        m_settings = PolygonalSelectorSettings{};
    }

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

        if (m_currentVertices.size() >= MAX_POLYGONAL_SELECTOR_VERTICES)
        {
            controller.updateInfo(new GuiDataWarning(QString("The creation of vertices is limited to %1 vertices per polygon.").arg(MAX_POLYGONAL_SELECTOR_VERTICES)));
            return (m_state = ContextState::waiting_for_input);
        }

        m_currentVertices.emplace_back(normalized);

        if (rCam)
        {
            m_lastSnapshot.view = rCam->getViewMatrix();
            m_lastSnapshot.proj = rCam->getProjMatrix();
            m_lastSnapshot.viewportWidth = click.width;
            m_lastSnapshot.viewportHeight = click.height;
            m_lastSnapshot.perspective = (rCam->getProjectionMode() == ProjectionMode::Perspective);
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
        if (m_settings.polygons.size() >= MAX_POLYGONAL_SELECTOR_POLYGONS)
        {
            controller.updateInfo(new GuiDataWarning(QString("The creation of polygons is limited to %1 polygons per filter.").arg(MAX_POLYGONAL_SELECTOR_POLYGONS)));
        }
        else
        {
            PolygonalSelectorPolygon polygon;
            m_settings.nextPolygonId = computeNextPolygonId(m_settings);
            polygon.name = QString("polygon_%1").arg(m_settings.nextPolygonId).toStdString();
            polygon.normalizedVertices = m_currentVertices;
            polygon.camera = m_lastSnapshot;

            ClippingAssembly clippingAssembly;
            controller.getGraphManager().getClippingAssembly(clippingAssembly, true, false);
            captureSnapshotClipping(clippingAssembly, polygon);

            controller.updateInfo(new GuiDataRenderPolygonalSelector(m_settings, SafePtr<CameraNode>(), m_currentVertices, true));

            m_settings.polygons.push_back(polygon);
            ++m_settings.nextPolygonId;
            m_settings.enabled = true;
            m_settings.pendingApply = true;
        }
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
