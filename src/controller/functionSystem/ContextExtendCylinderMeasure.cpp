#include "controller/functionSystem/ContextExtendCylinderMeasure.h"
#include "controller/controls/ControlFunction.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"

#include "pointCloudEngine/TlScanOverseer.h"

#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"

#include "models/3d/Graph/CylinderNode.h"
#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"

#include "utils/Logger.h"

#include <glm/gtx/quaternion.hpp>

#define M_PI           3.14159265358979323846  /* pi */

ContextExtendCylinder::ContextExtendCylinder(const ContextId& id)
	: ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_BIGCYLINDERFIT_START });
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_BIGCYLINDERFIT_FOURTH });
}

ContextExtendCylinder::~ContextExtendCylinder()
{
}

ContextState ContextExtendCylinder::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState  ContextExtendCylinder::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
	return m_state;
}

ContextState ContextExtendCylinder::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	OpenScanToolsGraphManager& graphManager = controller.getOpenScanToolsGraphManager();

	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));
	bool success = false;

    ClippingAssembly clippingAssembly;
	graphManager.getClippingAssembly(clippingAssembly, true, false);
	glm::dvec3 cylinderDirection, cylinderCenter;
	double cylinderRadius, radius(0.4);
	std::vector<double> heights(2);
	std::vector<glm::dvec3> seedPoints;
	seedPoints.push_back(0.5*(m_clickResults[0].position + m_clickResults[1].position));
	seedPoints.push_back(0.25*m_clickResults[0].position + 0.75*m_clickResults[1].position);
	seedPoints.push_back(0.75*m_clickResults[0].position + 0.25*m_clickResults[1].position);
    TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));
	for (int i = 0; i < 3; i++)
	{
		if (TlScanOverseer::getInstance().fitCylinder(seedPoints[i], radius, 0.002, cylinderRadius, cylinderDirection, cylinderCenter, FitCylinderMode::robust, clippingAssembly))
		{
			controller.updateInfo(new GuiDataTmpMessage(QString(TEXT_CYLINDER_FOUND).arg(2 * cylinderRadius, cylinderCenter.x, cylinderCenter.y, cylinderCenter.z),0));
			success = true;
			break;
		}
	}

	if(!success)
	{
		m_clickResults.clear();
		return waitForNextPoint(controller, TEXT_CYLINDER_NOT_FOUND);
	}
	FUNCLOG << "ContextFitCylinder launch" << LOGENDL;
	
	SafePtr<CylinderNode> cylinder = make_safe<CylinderNode>(cylinderRadius);
	WritePtr<CylinderNode> wCyl = cylinder.get();
	if (!wCyl)
	{
		m_clickResults.clear();
		return waitForNextPoint(controller);
	}
	
	wCyl->setDefaultData(controller);

	heights[0] = TlScanOverseer::computeHeight(m_clickResults[0].position, cylinderDirection, cylinderCenter);
	heights[1] = TlScanOverseer::computeHeight(m_clickResults[1].position, cylinderDirection, cylinderCenter);
	cylinderCenter=cylinderCenter + 0.5*(heights[0] + heights[1])*cylinderDirection;
	wCyl->setPosition(cylinderCenter);
	wCyl->setRotation(glm::dquat(glm::rotation(glm::dvec3(0.0, 0.0, 1.0), cylinderDirection)));

	wCyl->setLength(abs((heights[1]-heights[0])));

	controller.getControlListener()->notifyUIControl(new control::function::AddNodes(cylinder));

	controller.updateInfo(new GuiDataTmpMessage(TEXT_BIGCYLINDERFIT_START, 0));

	m_clickResults.clear();
	return waitForNextPoint(controller);
}

bool ContextExtendCylinder::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextExtendCylinder::getType() const
{
	return (ContextType::cylinder2ClickExtend);
}
