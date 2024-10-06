#include "controller/functionSystem/ContextPointToCylinderMeasure.h"
#include "controller/controls/ControlFunctionTag.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "controller/controls/ControlFunction.h"

#include "pointCloudEngine/TlScanOverseer.h"

#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"

#include "models/graph/PointToPipeMeasureNode.h"
#include "models/graph/CylinderNode.h"
#include "models/graph/GraphManager.hxx"

#include "utils/Logger.h"

#include <glm/gtx/quaternion.hpp>

ContextPointToCylinderMeasure::ContextPointToCylinderMeasure(const ContextId& id)
	: ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Tag, ElementType::Point}, TEXT_POINTTOPLANE_START });
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_POINTTOCYLINDER_START });
}

ContextPointToCylinderMeasure::~ContextPointToCylinderMeasure()
{
}

ContextState ContextPointToCylinderMeasure::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState  ContextPointToCylinderMeasure::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
	return (m_state);
}

ContextState ContextPointToCylinderMeasure::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));
	bool success = false;

	glm::dvec3 projectedPoint, projectedCylinderPoint, cylinderDirection, cylinderCenter;
	double cylinderRadius;

	GraphManager& graphManager = controller.getGraphManager();

    TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));
    ClippingAssembly clippingAssembly;

	graphManager.getClippingAssembly(clippingAssembly, true, false);
	if (TlScanOverseer::getInstance().pointToCylinderMeasure(m_clickResults[0].position, m_clickResults[1].position, projectedPoint, projectedCylinderPoint, cylinderRadius, cylinderDirection, cylinderCenter, clippingAssembly))
	{
		//create cylinder
		SafePtr<CylinderNode> cyl = make_safe<CylinderNode>(cylinderRadius);
		WritePtr<CylinderNode> wCyl = cyl.get();
		if (!wCyl)
		{
			m_clickResults.clear();
			return waitForNextPoint(controller);
		}

		wCyl->setDefaultData(controller);

		wCyl->setVisible(true);
		wCyl->setPosition(cylinderCenter);
		wCyl->setRotation(glm::dquat(glm::rotation(glm::dvec3(0.0, 0.0, 1.0), cylinderDirection)));
		double length;
		if (cylinderRadius < 0.015)
			length = 3 * cylinderRadius;
		else if (cylinderRadius > 0.075)
			length = cylinderRadius;
		else length = cylinderRadius * (2 * (0.015 - cylinderRadius) / 0.06 + 3);
		wCyl->setLength(abs(2 * length));

		controller.getControlListener()->notifyUIControl(new control::function::AddNodes(cyl));

		//create measure
		SafePtr<PointToPipeMeasureNode> measure = graphManager.createMeasureNode<PointToPipeMeasureNode>();
		WritePtr<PointToPipeMeasureNode> wMeasure = measure.get();
		if (!wMeasure)
		{
			m_clickResults.clear();
			return waitForNextPoint(controller);
		}

		wMeasure->setDefaultData(controller);

		wMeasure->setVisible(true);
		wMeasure->setPointToAxeDist((float)glm::length(m_clickResults[0].position - projectedPoint));
		glm::dvec3 pointToAxis = m_clickResults[0].position - projectedPoint;
		wMeasure->setPointToAxeVertical((float)abs(pointToAxis[2]));
		pointToAxis[2] = 0;
		wMeasure->setPointToAxeHorizontal((float)glm::length(pointToAxis));
		glm::dvec3 freeD = m_clickResults[0].position - projectedPoint;
		freeD /= glm::length(freeD);
		freeD = freeD*(glm::length(m_clickResults[0].position - projectedPoint) - cylinderRadius);
		wMeasure->setFreeD((float)glm::length(freeD));
		wMeasure->setFreeDistVertical((float)abs(freeD[2]));
		freeD[2] = 0;
		wMeasure->setFreeDistHorizontal((float)glm::length(freeD));

		wMeasure->setTotalFootprint((float)(glm::length(m_clickResults[0].position - projectedPoint) + cylinderRadius));

		wMeasure->setPipeDiameter((float)2 * cylinderRadius);
		wMeasure->setPipeCenter(projectedCylinderPoint);
		wMeasure->setPointCoord(m_clickResults[0].position);
		wMeasure->setProjPoint(projectedPoint);
		wMeasure->setPipeCenterToProj((float)glm::length(projectedCylinderPoint - projectedPoint));

		controller.getControlListener()->notifyUIControl(new control::function::AddNodes(measure));

	}

	m_clickResults.clear();
	return waitForNextPoint(controller);
}


bool ContextPointToCylinderMeasure::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextPointToCylinderMeasure::getType() const
{
	return (ContextType::pointToCylinder);
}
