#include "controller/functionSystem/ContextPickColor.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "models/graph/GraphManager.h"
#include "models/pointCloud/PointXYZIRGB.h"
#include "pointCloudEngine/OctreeRayTracing.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "tls_def.h"

#include <QObject>
#include <limits>

namespace
{
    constexpr double kMinPickRadius = 0.001;

    double computePickRadius(const Controller& controller, const ClickInfo& clickInfo, const glm::dvec3& point)
    {
        double height = std::max(1.0, static_cast<double>(clickInfo.height));
        double pointSize = controller.cgetContext().getRenderPointSize() + 2.0;
        double distance = glm::length(point - clickInfo.rayOrigin);
        double pixelWorldSize = clickInfo.heightAt1m * pointSize / height;
        return std::max(kMinPickRadius, distance * pixelWorldSize);
    }

    GeometricBox makePickBox(const glm::dvec3& center, double radius)
    {
        std::vector<glm::dvec3> corners;
        corners.reserve(8);
        corners.push_back({ center.x - radius, center.y - radius, center.z - radius });
        corners.push_back({ center.x + radius, center.y - radius, center.z - radius });
        corners.push_back({ center.x + radius, center.y + radius, center.z - radius });
        corners.push_back({ center.x - radius, center.y + radius, center.z - radius });
        corners.push_back({ center.x - radius, center.y - radius, center.z + radius });
        corners.push_back({ center.x + radius, center.y - radius, center.z + radius });
        corners.push_back({ center.x + radius, center.y + radius, center.z + radius });
        corners.push_back({ center.x - radius, center.y + radius, center.z + radius });
        GeometricBox box(corners);
        box.setRadius(radius);
        return box;
    }

    bool tryFindNearestColor(const Controller& controller, const ClickInfo& clickInfo, const glm::dvec3& point, PointXYZIRGB& outPoint)
    {
        double radius = computePickRadius(controller, clickInfo, point);
        ClippingAssembly clipAssembly;
        controller.cgetGraphManager().getClippingAssembly(clipAssembly, true, false);

        TlScanOverseer::setWorkingScansTransfo(controller.cgetGraphManager().getVisiblePointCloudInstances(clickInfo.panoramic, true, true));

        for (int attempt = 0; attempt < 3; ++attempt)
        {
            std::vector<PointXYZIRGB> points;
            GeometricBox box = makePickBox(point, radius);
            TlScanOverseer::getInstance().collectPointsInGeometricBox(box, clipAssembly, tls::ScanGuid(), points);
            if (!points.empty())
            {
                double bestDistance = std::numeric_limits<double>::max();
                for (const PointXYZIRGB& candidate : points)
                {
                    glm::dvec3 candidatePos(candidate.x, candidate.y, candidate.z);
                    double distance = glm::length(candidatePos - point);
                    if (distance < bestDistance)
                    {
                        bestDistance = distance;
                        outPoint = candidate;
                    }
                }
                return true;
            }

            radius *= 2.0;
        }

        return false;
    }
}

ContextPickColor::ContextPickColor(const ContextId& id)
    : ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, QObject::tr("Pick a point to read RGB") });
}

ContextPickColor::~ContextPickColor() = default;

ContextState ContextPickColor::start(Controller& controller)
{
    return ARayTracingContext::start(controller);
}

ContextState ContextPickColor::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextPickColor::launch(Controller& controller)
{
    ARayTracingContext::getNextPosition(controller);
    if (pointMissing())
        return waitForNextPoint(controller);

    const ClickResult& clickResult = m_clickResults[0];
    PointXYZIRGB pickedPoint{};
    bool hasRgb = tryFindNearestColor(controller, m_lastClickInfo, clickResult.position, pickedPoint);

    Color32 color = hasRgb ? Color32(pickedPoint.r, pickedPoint.g, pickedPoint.b, 255) : Color32(0, 0, 0, 0);
    controller.updateInfo(new GuiDataColorimetricFilterPickedColor(color, hasRgb));

    m_clickResults.clear();
    return waitForNextPoint(controller);
}

bool ContextPickColor::canAutoRelaunch() const
{
    return true;
}

ContextType ContextPickColor::getType() const
{
    return ContextType::pickColor;
}
