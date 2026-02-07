#include "controller/functionSystem/ContextPickTemperature.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "gui/UnitConverter.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "models/graph/GraphManager.h"
#include "models/pointCloud/PointXYZIRGB.h"
#include "pointCloudEngine/OctreeRayTracing.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "tls_def.h"
#include "utils/TemperatureScaleUtils.h"

#include <QObject>
#include <algorithm>
#include <cmath>
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

    bool tryFindRayTracedPoint(const Controller& controller, const ClickInfo& clickInfo, PointXYZIRGB& outPoint)
    {
        double height = std::max(1.0, static_cast<double>(clickInfo.height));
        double pointSize = controller.cgetContext().getRenderPointSize() + 2.0;
        bool isOrtho = (std::abs(clickInfo.fov) <= std::numeric_limits<double>::epsilon());
        double cosAngleThreshold = atan(clickInfo.heightAt1m * pointSize / (1.0 * height));
        cosAngleThreshold = isOrtho ? clickInfo.heightAt1m * pointSize / (1.0 * height) : cos(cosAngleThreshold);

        ClippingAssembly clipAssembly;
        controller.cgetGraphManager().getClippingAssembly(clipAssembly, true, false);

        TlScanOverseer::setWorkingScansTransfo(controller.cgetGraphManager().getVisiblePointCloudInstances(clickInfo.panoramic, true, true));
        glm::dvec3 bestPoint;
        std::string scanName;
        return TlScanOverseer::getInstance().rayTracingWithPoint(clickInfo.ray, clickInfo.rayOrigin, bestPoint, outPoint, cosAngleThreshold, clipAssembly, isOrtho, scanName);
    }
}

ContextPickTemperature::ContextPickTemperature(const ContextId& id)
    : ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, QObject::tr("Pick a point to read temperature") });
}

ContextPickTemperature::~ContextPickTemperature() = default;

ContextState ContextPickTemperature::start(Controller& controller)
{
    return ARayTracingContext::start(controller);
}

ContextState ContextPickTemperature::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextPickTemperature::launch(Controller& controller)
{
    ARayTracingContext::getNextPosition(controller);
    if (pointMissing())
        return waitForNextPoint(controller);

    const ClickResult& clickResult = m_clickResults[0];
    PointXYZIRGB pickedPoint{};
    bool hasRgb = tryFindRayTracedPoint(controller, m_lastClickInfo, pickedPoint);
    if (!hasRgb)
        hasRgb = tryFindNearestColor(controller, m_lastClickInfo, clickResult.position, pickedPoint);

    QString temperatureText = QStringLiteral("NA");
    TemperatureScaleData temperatureScale = controller.cgetGraphManager().getTemperatureScaleData();
    if (hasRgb && temperatureScale.isValid)
    {
        uint32_t key = makeTemperatureScaleKey(pickedPoint.r, pickedPoint.g, pickedPoint.b);
        auto it = temperatureScale.rgbToTemperature.find(key);
        if (it != temperatureScale.rgbToTemperature.end())
        {
            int digits = controller.cgetContext().m_unitUsage.displayedDigits;
            temperatureText = QStringLiteral("%1%2")
                                  .arg(QString::number(it->second, 'f', digits), UnitConverter::getTemperatureUnitText());
        }
    }

    QString rgbText = hasRgb
        ? QStringLiteral("%1,%2,%3").arg(pickedPoint.r).arg(pickedPoint.g).arg(pickedPoint.b)
        : QStringLiteral("NA");

    QString message = QStringLiteral("Point temperature: %1\nRGB: %2").arg(temperatureText, rgbText);
    controller.updateInfo(new GuiDataInfo(message, false));

    m_clickResults.clear();
    return waitForNextPoint(controller);
}

bool ContextPickTemperature::canAutoRelaunch() const
{
    return true;
}

ContextType ContextPickTemperature::getType() const
{
    return ContextType::pickTemperature;
}
