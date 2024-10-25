#include "controller/functionSystem/ContextTrajectory.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "controller/Controller.h"
#include "controller/ControlListener.h" // forward declaration
#include "controller/controls/ControlFunction.h"

#include "models/graph/GraphManager.h"
#include "models/graph/CylinderNode.h"

#include <glm/gtx/quaternion.hpp>

ContextTrajectory::ContextTrajectory(const ContextId& id)
	: ARayTracingContext(id)
{
	resetClickUsages();
}

ContextTrajectory::~ContextTrajectory()
{
}

void ContextTrajectory::resetClickUsages()
{
	m_usages.clear();
	m_clickResults.clear();
	m_usages.push_back({ true, {ElementType::Point, ElementType::Tag, ElementType::Sphere}, TEXT_POINTTOCYLINDER_START });
	m_usages.push_back({ true, {ElementType::Point, ElementType::Tag, ElementType::Sphere}, TEXT_BIGCYLINDERFIT_FOURTH });

}

ContextState ContextTrajectory::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState ContextTrajectory::feedMessage(IMessage* message, Controller& controller)
{
	return ARayTracingContext::feedMessage(message, controller);
}

ContextState ContextTrajectory::launch(Controller& controller)
{
	// --- Ray Tracing ---
	ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
	// -!- Ray Tracing -!-



	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));

	GraphManager& graphManager = controller.getGraphManager();

	glm::dvec3 cylinderDirection, cylinderCenter;
	double cylinderRadius(0.005);
	cylinderDirection = m_clickResults[1].position - m_clickResults[0].position;
	cylinderDirection /= glm::length(cylinderDirection);
	cylinderCenter = 0.5 * (m_clickResults[0].position + m_clickResults[1].position);
	SafePtr<CylinderNode> cylinderNode = make_safe<CylinderNode>(cylinderRadius);
	WritePtr<CylinderNode> wCyl = cylinderNode.get();

	wCyl->setDefaultData(controller);

	TransformationModule mod;
	mod.setRotation(glm::dquat(glm::rotation(glm::dvec3(0.0, 0.0, 1.0), cylinderDirection)));
	mod.setPosition(cylinderCenter);
	wCyl->setTransformationModule(mod);
	double length = glm::length(m_clickResults[0].position - m_clickResults[1].position);
	wCyl->setLength(abs(length));

	controller.getControlListener()->notifyUIControl(new control::function::AddNodes(cylinderNode));
	return waitForNextPoint(controller);
}

bool ContextTrajectory::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextTrajectory::getType() const
{
	return (ContextType::trajectory);
}
