#include "controller/functionSystem/ContextPickColorimetric.h"

#include "controller/Controller.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "models/graph/GraphManager.h"
#include "models/pointCloud/PointXYZIRGB.h"
#include "pointCloudEngine/OctreeRayTracing.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "tls_def.h"

#include <QObject>
#include <algorithm>
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

    bool tryFindNearestPoint(const Controller& controller, const ClickInfo& clickInfo, const glm::dvec3& point, PointXYZIRGB& outPoint)
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

ContextPickColorimetric::ContextPickColorimetric(const ContextId& id)
    : ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, QObject::tr("Pick a point to read raw color") });
}

ContextPickColorimetric::~ContextPickColorimetric() = default;

ContextState ContextPickColorimetric::start(Controller& controller)
{
    return ARayTracingContext::start(controller);
}

ContextState ContextPickColorimetric::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextPickColorimetric::launch(Controller& controller)
{
    ARayTracingContext::getNextPosition(controller);
    if (pointMissing())
        return waitForNextPoint(controller);

    const ClickResult& clickResult = m_clickResults[0];
    PointXYZIRGB pickedPoint{};
    bool hasPoint = tryFindNearestPoint(controller, m_lastClickInfo, clickResult.position, pickedPoint);

    if (hasPoint)
    {
        Color32 color(pickedPoint.r, pickedPoint.g, pickedPoint.b);
        controller.updateInfo(new GuiDataColorimetricFilterPickValue(color, pickedPoint.i));
    }

    m_clickResults.clear();
    return waitForNextPoint(controller);
}

bool ContextPickColorimetric::canAutoRelaunch() const
{
    return true;
}

ContextType ContextPickColorimetric::getType() const
{
    return ContextType::pickColorimetricFilter;
}
