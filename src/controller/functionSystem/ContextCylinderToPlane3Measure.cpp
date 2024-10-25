#include "controller/functionSystem/ContextCylinderToPlane3Measure.h"
#include "controller/Controller.h"
#include "controller/ControlListener.h" // forward declaration
#include "controller/controls/ControlFunction.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "pointCloudEngine/MeasureClass.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"

#include "models/graph/CylinderNode.h"
#include "models/graph/PipeToPlaneMeasureNode.h"
#include "models/graph/GraphManager.hxx"

#include "utils/Logger.h"

#include <glm/gtx/quaternion.hpp>

ContextCylinderToPlane3Measure::ContextCylinderToPlane3Measure(const ContextId& id)
	: ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_CYLINDERTOPLANE_START });
    m_usages.push_back({ true, {ElementType::Tag, ElementType::Point}, TEXT_PLANE3_FIRST });
    m_usages.push_back({ true, {ElementType::Tag, ElementType::Point}, TEXT_PLANE3_SECOND });
    m_usages.push_back({ true, {ElementType::Tag, ElementType::Point}, TEXT_PLANE3_THIRD });
}

ContextCylinderToPlane3Measure::~ContextCylinderToPlane3Measure()
{
}

ContextState ContextCylinderToPlane3Measure::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState ContextCylinderToPlane3Measure::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextCylinderToPlane3Measure::launch(Controller& controller)
{
    // --- Ray Tracing ---
	if (!ARayTracingContext::getNextPosition(controller))
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	GraphManager& graphManager = controller.getGraphManager();

	if(m_clickResults.size() == 1){
		controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));
		if (std::isnan(m_clickResults[0].position.x))
		{
			FUNCLOG << "picking nan detected" << LOGENDL;
			m_clickResults.clear();
			return waitForNextPoint(controller, TEXT_CYLINDER_NOT_FOUND);
		}
		FUNCLOG << "ContextPointToCylinderMeasure feedClick " << m_clickResults[0].position.x << " " << m_clickResults[0].position.y << " " << m_clickResults[0].position.z << LOGENDL;

		ClippingAssembly clippingAssembly;
		graphManager.getClippingAssembly(clippingAssembly, true, false);

		TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));
		if (!TlScanOverseer::getInstance().fitCylinder(m_clickResults[0].position, 0.4, 0.002, m_cylinderRadius, m_cylinderDirection, m_cylinderCenter, FitCylinderMode::robust, clippingAssembly))
		{
			m_clickResults.clear();
			return waitForNextPoint(controller, TEXT_CYLINDER_NOT_FOUND);
		}
	}
	if(m_clickResults.size() < m_usages.size())
		return waitForNextPoint(controller);

    // Compute plane
	std::vector<double> plane;
    TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));

	std::vector<glm::dvec3> planePoints;
	planePoints.push_back(m_clickResults[1].position);
	planePoints.push_back(m_clickResults[2].position);
	planePoints.push_back(m_clickResults[3].position);
	TlScanOverseer::getInstance().fitPlane3Points(planePoints, plane);
	glm::dvec3 projectedPlanePoint, cylinderAxisPoint, normalVector;
	cylinderAxisPoint = MeasureClass::projectPointToLine(m_clickResults[0].position, m_cylinderDirection, m_cylinderCenter);
	projectedPlanePoint = MeasureClass::projectPointToPlane(cylinderAxisPoint, plane);
	normalVector = glm::dvec3(plane[0], plane[1], plane[2]);
	normalVector /= glm::length(normalVector);

	//create cylinder
	SafePtr<CylinderNode> cylinder = make_safe<CylinderNode>(m_cylinderRadius);
	WritePtr<CylinderNode> wCyl = cylinder.get();
	if (!wCyl)
	{
		assert(false);
		m_clickResults.clear();
		return waitForNextPoint(controller);
	}

	wCyl->setDefaultData(controller);

	wCyl->setVisible(true);
	TransformationModule mod;
	mod.setPosition(m_cylinderCenter);
	mod.setRotation(glm::dquat(glm::rotation(glm::dvec3(0.0, 0.0, 1.0), m_cylinderDirection)));
	wCyl->setTransformationModule(mod);
	double length;
	if (m_cylinderRadius < 0.015)
		length = 3 * m_cylinderRadius;
	else if (m_cylinderRadius > 0.075)
		length = m_cylinderRadius;
	else length = m_cylinderRadius * (2 * (0.015 - m_cylinderRadius) / 0.06 + 3);
	wCyl->setLength(abs(2 * length));

	controller.getControlListener()->notifyUIControl(new control::function::AddNodes(cylinder));

	//create measure
	SafePtr<PipeToPlaneMeasureNode> measure = graphManager.createMeasureNode<PipeToPlaneMeasureNode>();
	WritePtr<PipeToPlaneMeasureNode> wMeasure = measure.get();
	if (!wMeasure)
	{
		assert(false);
		m_clickResults.clear();
		return waitForNextPoint(controller);
	}
	wMeasure->setDefaultData(controller);
	wMeasure->setCenterToPlaneDist((float)glm::length(cylinderAxisPoint - projectedPlanePoint));
	glm::dvec3 planeCenter = cylinderAxisPoint - projectedPlanePoint;
	wMeasure->setPlaneCenterVertical((float)abs(planeCenter[2]));
	planeCenter[2] = 0;
	wMeasure->setPlaneCenterHorizontal((float)glm::length(planeCenter));
	wMeasure->setPipeDiameter(2.f *  m_cylinderRadius);
	wMeasure->setPipeCenter(m_cylinderCenter);
	wMeasure->setPointOnPlane((1.0/3.0) * (m_clickResults[1].position + m_clickResults[2].position + m_clickResults[3].position));
	wMeasure->setProjPoint(projectedPlanePoint);
	wMeasure->setNormalOnPlane(normalVector);
	wMeasure->setPointOnPlaneToProj((float)glm::length(projectedPlanePoint - m_clickResults[1].position));
	glm::dvec3 freeD = m_cylinderCenter - projectedPlanePoint;
	freeD /= glm::length(freeD);
	freeD = freeD * (glm::length(m_cylinderCenter - projectedPlanePoint) - m_cylinderRadius);
	wMeasure->setFreeDist((float)glm::length(freeD));
	wMeasure->setFreeDistVertical((float)abs(freeD[2]));
	freeD[2] = 0;
	wMeasure->setFreeDistHorizontal((float)glm::length(freeD));
	wMeasure->setTotalFootprint((float)(glm::length(m_cylinderCenter - projectedPlanePoint) + m_cylinderRadius));
	wMeasure->setVisible(true);

	controller.getControlListener()->notifyUIControl(new control::function::AddNodes(measure));

	controller.updateInfo(new GuiDataTmpMessage(TEXT_CYLINDERTOPLANE_START));

	m_clickResults.clear();
	return waitForNextPoint(controller);
}


bool ContextCylinderToPlane3Measure::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextCylinderToPlane3Measure::getType() const
{
	return (ContextType::cylinderToPlane3);
}