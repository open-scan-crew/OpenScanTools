#include "controller/functionSystem/ContextFitTorus.h"
#include "controller/controls/ControlFunction.h"

#include "models/3d/Graph/TagNode.h"
#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"
#include "pointCloudEngine/TlScanOverseer.h"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "models/3d/Graph/TorusNode.h"

#include "utils/Logger.h"
#include <glm/gtx/quaternion.hpp>

ContextFitTorus::ContextFitTorus(const ContextId& id)
	: ARayTracingContext(id)
{
	m_usages.clear();
	m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, "" });
}

ContextFitTorus::~ContextFitTorus()
{
}

ContextState ContextFitTorus::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState  ContextFitTorus::feedMessage(IMessage* message, Controller& controller)
{
	ARayTracingContext::feedMessage(message, controller);
	return m_state;
}

ContextState ContextFitTorus::launch(Controller& controller)
{
	// --- Ray Tracing ---
	ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
	// -!- Ray Tracing -!-

	FUNCLOG << "ContextFitCylinder launch" << LOGENDL;
	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));
	bool success = false;

	OpenScanToolsGraphManager& graphManager = controller.getOpenScanToolsGraphManager();

	ClippingAssembly clippingAssembly;
	graphManager.getClippingAssembly(clippingAssembly, true, false);

	
	TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));
	std::vector<glm::dvec3> dataPoints(0),testPoints(0);
	glm::dvec3 torusCenter,axis;
	double principalRadius, pipeRadius;
	if (!TlScanOverseer::getInstance().torusFitFromCirclesPrep(m_clickResults[0].position, clippingAssembly, torusCenter, principalRadius, pipeRadius, axis))
	{
		m_clickResults.clear();
		return waitForNextPoint(controller);
	}
	
	glm::dvec3 dirX, dirY;
	OctreeRayTracing::completeVectorToOrthonormalBasis(axis, dirX, dirY);
	// create full torus

	TransformationModule mod2;
	//mod2.setPosition(elbowPoints[size-2]);
	mod2.setPosition(torusCenter);
	mod2.setScale(glm::vec3(principalRadius+pipeRadius));
	//mod2.setScale(glm::dvec3(0.5*(mainRadius + tubeRadius), 0.5*(mainRadius + tubeRadius), tubeRadius));
	double sign1(1.0), sign2(1.0);
	/*if (glm::dot(m_cylinderDirectionsModif[size - 2], m_cylinderCentersModif[size - 2] - elbowEdges[size - 2][0]) > 0)
	{
		sign1 = -1;
	}
	else { sign1 = +1; }
	if (glm::dot(m_cylinderDirectionsModif[size - 1], m_cylinderCentersModif[size - 1] - elbowEdges[size - 2][1]) > 0)
	{
		sign2 = 1;
	}
	else { sign2 = -1; }*/


	glm::dquat quat1 = glm::rotation(glm::dvec3(0.0, 1.0, 0.0), sign1 * dirX);
	glm::dvec3 up = glm::normalize(glm::cross(sign1 * dirX, sign2 * dirY));
	glm::dvec3 newUp = quat1 * glm::dvec3(0.0, 0.0, 1.0);
	glm::dquat quat2 = glm::rotation(newUp, up);
	mod2.setRotation(quat2 * quat1);

	double mainAngle(6.28318530718);
	SafePtr<TorusNode> torusPtr = make_safe<TorusNode>(mainAngle, principalRadius, pipeRadius, 0.0);
	WritePtr<TorusNode> writeTorus = torusPtr.get();
	if (writeTorus)
	{
		writeTorus->setDefaultData(controller);
		writeTorus->setTransformationModule(mod2);
		//writeTorus->setInsulationRadius(m_options.insulatedThickness);

		controller.getControlListener()->notifyUIControl(new control::function::AddNodes(torusPtr));

		
	}

	m_clickResults.clear();
	return waitForNextPoint(controller);
}

bool ContextFitTorus::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextFitTorus::getType() const
{
	return (ContextType::fitTorus);
}
