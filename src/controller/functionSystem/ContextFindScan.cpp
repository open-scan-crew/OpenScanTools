#include "controller/functionSystem/ContextFindScan.h"
#include "controller/Controller.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "models/graph/CameraNode.h"
#include "models/graph/GraphManager.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "pointCloudEngine/OctreeRayTracing.h"
#include "utils/TemperatureScaleParser.h"
#include "utils/Utils.h"

#include <limits>


ContextFindScan::ContextFindScan(const ContextId& id)
	: ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_POINTTOPLANE_START });
}

ContextFindScan::~ContextFindScan()
{
}

ContextState ContextFindScan::start(Controller&  controller)
{
	return ARayTracingContext::start(controller);
}

ContextState ContextFindScan::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextFindScan::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	QString scanFound = QString(TEXT_CONTEXT_NO_SCAN_FOUND);
	if (!pointMissing())
		scanFound = QString::fromStdString(m_clickResults[0].scanName);

	controller.updateInfo(new GuiDataInfo(TEXT_CONTEXT_FIND_SCAN_MODAL.arg(scanFound), false));

    m_clickResults.clear();
	return waitForNextPoint(controller);
}

bool ContextFindScan::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextFindScan::getType() const
{
	return (ContextType::findScan);
}

ContextPickTemperature::ContextPickTemperature(const ContextId& id)
	: ARayTracingContext(id)
{
	m_usages.push_back({ true, { ElementType::Point, ElementType::Tag }, TEXT_CONTEXT_TEMPERATURE_PICK_START });
}

ContextPickTemperature::~ContextPickTemperature()
{
}

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

	double temperature = 0.0;
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	bool hasTemperature = findTemperatureFromPick(controller, temperature, r, g, b);

	QString temperatureText = hasTemperature ? QString::number(temperature, 'f', controller.getContext().m_unitUsage.displayedDigits) + " °C" : QStringLiteral("NA");
	QString message = QStringLiteral("Point temperature: %1\nRGB: %2, %3, %4")
		.arg(temperatureText)
		.arg(r)
		.arg(g)
		.arg(b);

	controller.updateInfo(new GuiDataInfo(message, false));

	m_clickResults.clear();
	return waitForNextPoint(controller);
}

bool ContextPickTemperature::findTemperatureFromPick(Controller& controller, double& temperature, uint8_t& r, uint8_t& g, uint8_t& b)
{
	if (m_clickResults.empty())
		return false;

	ReadPtr<CameraNode> rCam = controller.getGraphManager().getCameraNode().cget();
	if (!rCam)
		return false;

	const RampScale& rampScale = rCam->getDisplayParameters().m_rampScale;
	const bool hasScaleFile = !rampScale.temperatureScaleFile.empty();

	const std::string& scanName = m_clickResults[0].scanName;
	std::vector<tls::PointCloudInstance> instances = controller.getGraphManager().getVisiblePointCloudInstances(m_panoramic, true, true);

	const tls::PointCloudInstance* targetInstance = nullptr;
	for (const tls::PointCloudInstance& instance : instances)
	{
		if (Utils::to_utf8(instance.header.name) == scanName)
		{
			targetInstance = &instance;
			break;
		}
	}

	if (!targetInstance)
		return false;

	ClippingAssembly clipAssembly;
	controller.getGraphManager().getClippingAssembly(clipAssembly, true, false);

	const glm::dvec3& pickedPoint = m_clickResults[0].position;
	std::vector<PointXYZIRGB> points;

	auto collectPoints = [&](double radius)
	{
		std::vector<glm::dvec3> corners;
		corners.reserve(8);
		corners.push_back(pickedPoint + glm::dvec3(-radius, -radius, -radius));
		corners.push_back(pickedPoint + glm::dvec3(radius, -radius, -radius));
		corners.push_back(pickedPoint + glm::dvec3(-radius, radius, -radius));
		corners.push_back(pickedPoint + glm::dvec3(-radius, -radius, radius));
		corners.push_back(pickedPoint + glm::dvec3(radius, radius, -radius));
		corners.push_back(pickedPoint + glm::dvec3(radius, -radius, radius));
		corners.push_back(pickedPoint + glm::dvec3(-radius, radius, radius));
		corners.push_back(pickedPoint + glm::dvec3(radius, radius, radius));

		GeometricBox box(corners);
		TlScanOverseer::getInstance().collectPointsInGeometricBoxForScan(targetInstance->header.guid, box, targetInstance->transfo, clipAssembly, points);
	};

	collectPoints(0.01);
	if (points.empty())
		collectPoints(0.05);
	if (points.empty())
		return false;

	double bestDistance = std::numeric_limits<double>::max();
	PointXYZIRGB bestPoint = points.front();
	for (const PointXYZIRGB& pt : points)
	{
		glm::dvec3 pos(pt.x, pt.y, pt.z);
		double distance = glm::length(pos - pickedPoint);
		if (distance < bestDistance)
		{
			bestDistance = distance;
			bestPoint = pt;
		}
	}

	r = bestPoint.r;
	g = bestPoint.g;
	b = bestPoint.b;

	if (!hasScaleFile)
		return false;

	temperature_scale::TemperatureScaleData scaleData;
	std::string error;
	if (!temperature_scale::getTemperatureScaleData(rampScale.temperatureScaleFile, scaleData, &error))
		return false;

	uint32_t key = temperature_scale::makeRgbKey(r, g, b);
	auto it = scaleData.lookup.find(key);
	if (it == scaleData.lookup.end())
		return false;

	temperature = it->second;
	return true;
}

bool ContextPickTemperature::canAutoRelaunch() const
{
	return true;
}

ContextType ContextPickTemperature::getType() const
{
	return ContextType::pickTemperature;
}
