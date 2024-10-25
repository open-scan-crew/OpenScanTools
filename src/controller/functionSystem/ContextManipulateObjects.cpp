#include "controller/functionSystem/ContextManipulateObjects.h"
#include "controller/controls/ControlObject3DEdition.h"
#include "controller/Controller.h"
#include "controller/ControlListener.h" // forward declaration
#include "models/graph/GraphManager.h"
#include "models/graph/ManipulatorNode.h"
#include "utils/Logger.h"

#include "gui/texts/ContextTexts.hpp"
#include "gui/GuiData/GuiDataMessages.h"

#include "utils/math/glm_extended.h"

ContextManipulateObjects::ContextManipulateObjects(const ContextId& id)
	: ARayTracingContext(id)
{
}

ContextManipulateObjects::~ContextManipulateObjects()
{}

ContextState ContextManipulateObjects::start(Controller& controller)
{
	std::unordered_set<SafePtr<AGraphNode>> selects = controller.getGraphManager().getNodesByTypes(ManipulatorNode::getManipulableTypes(), ObjectStatusFilter::SELECTED);
	if (selects.empty())
	{
		controller.updateInfo(new GuiDataWarning(TEXT_NO_OBJECT_SELECTED));
		return ARayTracingContext::abort(controller);
	}

	m_toMove = selects;
    ARayTracingContext::start(controller);
	return m_state = ContextState::waiting_for_input;
}

ContextState ContextManipulateObjects::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    if (message->getType() == IMessage::MessageType::MANIPULATE)
    {
        ManipulateMessage* manipMsg = static_cast<ManipulateMessage*>(message);
		m_rotate = manipMsg->m_rotate;
		m_zmove = manipMsg->m_zmove;

		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_MANIP_OBJ_BASE_FIRST });
		if (m_rotate)
		{
			m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_MANIP_OBJ_BASE_SECOND });
			if (m_zmove == ZMovement::Default)
				m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_MANIP_OBJ_BASE_THIRD });

			m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_MANIP_OBJ_TARGET_FIRST });
			m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_MANIP_OBJ_TARGET_SECOND });

			if (m_zmove == ZMovement::Default)
				m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_MANIP_OBJ_TARGET_THIRD });
		}
		else
		{
			m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_MANIP_OBJ_TARGET_FIRST });
		}

        m_state = ContextState::ready_for_using;
    }
    return (m_state);
}

ContextState ContextManipulateObjects::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-
	FUNCLOG << "ContextManipulateObjects launch" << LOGENDL; 

	if (m_rotate)
	{
		int ind = 0;
		glm::dvec3 B1 = m_clickResults[ind++].position;
		glm::dvec3 B2 = m_clickResults[ind++].position;
		glm::dvec3 B3;
		if (m_zmove == ZMovement::Lock)
			B3 = B1 + glm::dvec3(0., 0., 1.);
		else
			B3 = m_clickResults[ind++].position;

		glm::dvec3 T1 = m_clickResults[ind++].position;
		glm::dvec3 T2 = m_clickResults[ind++].position;
		glm::dvec3 T3;
		if (m_zmove == ZMovement::Lock)
			T3 = T1 + glm::dvec3(0., 0., 1.);
		else
			T3 = m_clickResults[ind++].position;

		glm::dvec3 N1 = glm::cross(B2 - B1, B3 - B1);
		glm::dvec3 N2 = glm::cross(T2 - T1, T3 - T1);

		if (glm::dot(N1, N2) < 0)
			N2 = -N2;
		double angle = glm_extended::angleBetweenDV3(N1, N2);
		glm::dvec3 pivot = glm::normalize(glm::cross(N1, N2));


		std::unordered_map<SafePtr<AGraphNode>, glm::dquat> newRotations;
		for (const SafePtr<AGraphNode>& toMove : m_toMove)
		{
			WritePtr<AGraphNode> wToMove = toMove.get();
			if (!wToMove)
				continue;
			glm::dquat quaternion = glm::normalize(glm::angleAxis(angle,pivot));

			newRotations[toMove] = quaternion * wToMove->getOrientation();
		}

		controller.getControlListener()->notifyUIControl(new control::object3DEdition::SetRotation(newRotations));
	}
	else
	{
		glm::dvec3 translation = m_clickResults[1].position - m_clickResults[0].position;
		if (m_zmove == ZMovement::Lock)
			translation.z = 0.;

		if (m_zmove == ZMovement::Along)
		{
			translation.x = 0.;
			translation.y = 0.;
		}

		std::unordered_map<SafePtr<AGraphNode>, glm::dvec3> newCenters;
		for (const SafePtr<AGraphNode>& toMove : m_toMove)
		{
			WritePtr<AGraphNode> wToMove = toMove.get();
			if (!wToMove)
				continue;

			newCenters[toMove] = wToMove->getCenter() + translation;
		}

		controller.getControlListener()->notifyUIControl(new control::object3DEdition::SetCenter(newCenters));
	}

	m_clickResults.clear();
	return ARayTracingContext::validate(controller);
}

bool ContextManipulateObjects::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextManipulateObjects::getType() const
{
	return (ContextType::tagMove);
}
