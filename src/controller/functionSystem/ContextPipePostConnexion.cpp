#include "controller/functionSystem/ContextPipePostConnexion.h"
#include "controller/controls/ControlObject3DEdition.h"
#include "controller/controls/ControlCylinderEdition.h"
#include "controller/controls/ControlDataEdition.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/Texts.hpp"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "pointCloudEngine/TlScanOverseer.h"

#include "models/graph/CylinderNode.h"
#include "models/graph/TorusNode.h"

#include "controller/controls/ControlFunctionClipping.h"
#include "utils/Logger.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "controller/messages/PipeMessage.h"
#include "controller/controls/ControlSpecial.h"
#include "controller/controls/ControlFunction.h"
#include "controller/controls/ControlMetaControl.h"

#include "models/graph/GraphManager.hxx"
#include "models/graph/ClusterNode.h"


#include <glm/gtx/quaternion.hpp>

ContextPipePostConnexion::ContextPipePostConnexion(const ContextId& id)
	: AContext(id)
{}

ContextPipePostConnexion::~ContextPipePostConnexion()
{}

ContextState ContextPipePostConnexion::start(Controller& controller)
{
	controller.getControlListener()->notifyUIControl(new control::meta::ControlStartMetaControl());
	return m_state = ContextState::waiting_for_input;
}

ContextState ContextPipePostConnexion::feedMessage(IMessage* message, Controller& controller)
{
	switch (message->getType())
	{
	case IMessage::MessageType::PIPECONNECTIONMESSAGE:
	{
		PipeConnetionMessage* pipeConnectionMessage = static_cast<PipeConnetionMessage*>(message);
		m_RonDext = pipeConnectionMessage->getRonDext();
		m_angleConstraints = pipeConnectionMessage->getAngleConstraints();
		m_keepDiameter = pipeConnectionMessage->getKeepDiameter();
		m_state = ContextState::ready_for_using;
		break;
	}
	default:
		break;
	}
	return (m_state);
}

ContextState ContextPipePostConnexion::launch(Controller& controller)
{
	GraphManager& graphManager = controller.getGraphManager();

	m_state = ContextState::running;
	std::unordered_set<SafePtr<AGraphNode>> cylindersData = graphManager.getNodesByTypes({ ElementType::Cylinder }, ObjectStatusFilter::SELECTED);
	std::vector<SafePtr<AGraphNode>> orderedData;

	if (cylindersData.empty())
		return abort(controller);

	std::vector<SafePtr<CylinderNode>> cylinders;
	std::map<SafePtr<AGraphNode>, std::vector<SafePtr<CylinderNode>>> pipingsLines;

	SafePtr<AGraphNode> maxPiping;
	size_t maxSize = 0;

	for (const SafePtr<AGraphNode>& data : cylindersData)
	{
		SafePtr<CylinderNode> cyl = static_pointer_cast<CylinderNode>(data);
		/*SafePtr<AGraphNode> piping = AGraphNode::getOwningParent(cyl, TreeType::Piping);
		if (pipingsLines.find(piping) != pipingsLines.end())
			pipingsLines[piping].push_back(cyl);
		else
			pipingsLines[piping] = { cyl };

		if (piping && maxSize < pipingsLines[piping].size())
		{
			maxPiping = piping;
			maxSize = pipingsLines[piping].size();
		}*/
		cylinders.push_back(cyl);
	}

	/*if (pipingsLines.find(SafePtr<AGraphNode>()) == pipingsLines.end())
		return abort(controller); 

	cylinders.insert(cylinders.begin(), pipingsLines[maxPiping].begin(), pipingsLines[maxPiping].end());
	if(maxPiping && pipingsLines.find(SafePtr<AGraphNode>()) != pipingsLines.end())
		cylinders.insert(cylinders.begin(), pipingsLines[SafePtr<AGraphNode>()].begin(), pipingsLines[SafePtr<AGraphNode>()].end());*/
	if (cylinders.empty())
		return abort(controller);
		
	std::vector<glm::dvec3> cylinderCenters,cylinderDirections;
	std::vector<double> cylinderLengths, cylinderRadii;
	for (int i = 0; i < (int)cylinders.size(); i++)
	{
		ReadPtr<CylinderNode> rCyl = cylinders[i].cget();
		orderedData.push_back(cylinders[i]);

		double radius= rCyl->getRadius();
		double length = rCyl->getLength();
		glm::dmat4 transfo = rCyl->getTransformation();
		glm::dvec3 dir = transfo * glm::dvec4(0.0, 0.0, 1.0, 0.0);
		dir = dir / glm::length(dir);
		glm::dvec3 center = transfo * glm::dvec4(0.0, 0.0, 0.0, 1.0);
		cylinderCenters.push_back(center);
		if (m_keepDiameter && (cylinderRadii.size() > 0))

			cylinderRadii.push_back(cylinderRadii[0]);	
		else
			cylinderRadii.push_back(radius);
		cylinderLengths.push_back(length);
		cylinderDirections.push_back(dir);
	}
	std::vector<int> order;
	bool success = TlScanOverseer::getInstance().arrangeCylindersInLine(cylinderCenters, cylinderDirections, cylinderLengths, cylinderRadii, order);

	if (success)
	{
		//create pipes and elbows
		std::vector<std::vector<glm::dvec3>> elbowEdges;
		std::vector<glm::dvec3>elbowPoints, elbowCenters;
		std::vector<double> angleModifs;
		std::vector<LineConnectionType> connectionType;
			
		bool globalSuccess = TlScanOverseer::getInstance().applyConstraints(cylinderCenters, cylinderDirections, cylinderLengths, angleModifs, elbowPoints, connectionType, m_RonDext, cylinderRadii, elbowEdges, m_angleConstraints, elbowCenters);
		std::unordered_set<SafePtr<AGraphNode>> lineObjects;


		if (!globalSuccess)
		{
			FUNCLOG << "ContextFitCylinder abort" << LOGENDL;
			return abort(controller);
		}
		else
		{
			for (int i = 0; i < (int)cylinderCenters.size(); i++)
			{
				SafePtr<CylinderNode> cylinder = static_pointer_cast<CylinderNode>(orderedData[order[i]]);

				// Set pipe standard
				if (controller.getContext().getCurrentStandard(StandardType::Pipe))
					controller.getControlListener()->notifyUIControl(new control::cylinderEdition::SetStandard(cylinder, controller.getContext().getCurrentStandard(StandardType::Pipe)));
				
				TransformationModule mod;
				mod.setPosition(cylinderCenters[i]);
				mod.setRotation(glm::dquat(glm::rotation(glm::dvec3(0.0, 0.0, 1.0), cylinderDirections[i])));

				controller.getControlListener()->notifyUIControl(new control::object3DEdition::SetTransformation(cylinder, mod));
				controller.getControlListener()->notifyUIControl(new control::cylinderEdition::SetForcedRadius(cylinder, cylinderRadii[i]));
				controller.getControlListener()->notifyUIControl(new control::cylinderEdition::SetLength(cylinder, cylinderLengths[i]));

				//controller.getControlListener()->notifyUIControl(new control::dataEdition::SetDescription(cylinder->getId(), L"angle modification : " + std::to_wstring(acos(angleModifs[i]) * 180 / 3.14159) + L"°"));

				lineObjects.insert(cylinder);

				if (i < (int)cylinderCenters.size() - 1)
				{
					if (!(connectionType[i]==LineConnectionType::elbow))
					{
						continue;
					}
					double sign1, sign2;
					if (glm::dot(cylinderDirections[i], cylinderCenters[i] - elbowEdges[i][0]) > 0)
					{
						sign1 = -1;
					}
					else { sign1 = +1; }
					if (glm::dot(cylinderDirections[i + 1], cylinderCenters[i + 1] - elbowEdges[i][1]) > 0)
					{
						sign2 = 1;
					}
					else
					{
						sign2 = -1;
					}
					double elbowAngleSign(1);
					elbowAngleSign = sign1 * sign2;
					double mainAngle(acos(elbowAngleSign* (glm::dot(cylinderDirections[i], cylinderDirections[i + 1]))));
					double tubeRadius(std::max(cylinderRadii[i], cylinderRadii[i + 1]));
					double mainRadius(m_RonDext*tubeRadius * 2);

					double insulatedRadius = controller.getContext().getPipeDetectionOptions().insulatedThickness;

					TransformationModule mod;

					mod.setPosition(elbowCenters[i]);
					mod.setScale(glm::vec3(mainRadius + (tubeRadius - insulatedRadius)));
					//mod.setScale(glm::dvec3(0.5*(mainRadius + tubeRadius), 0.5*(mainRadius + tubeRadius), tubeRadius));

					glm::dquat quat1 = glm::rotation(glm::dvec3(0.0, 1.0, 0.0), sign1 * cylinderDirections[i]);
					glm::dvec3 up = glm::normalize(glm::cross(sign1 * cylinderDirections[i], sign2 * cylinderDirections[i + 1]));
					glm::dvec3 newUp = quat1 * glm::dvec3(0.0, 0.0, 1.0);
					glm::dquat quat2 = glm::rotation(newUp, up);
					mod.setRotation(quat2 * quat1);
					//mod.setRotation(quatRotation);
					//mod.setRotation(glm::rotation(glm::dvec3(0.0, 0.0, 1.0), glm::normalize(glm::cross(m_cylinderDirections[i], m_cylinderDirections[i + 1]))));

					SafePtr<TorusNode> torusPtr = make_safe<TorusNode>(mainAngle, mainRadius, tubeRadius, insulatedRadius);
					WritePtr<TorusNode> writeTorus = torusPtr.get();

					if (writeTorus)
					{
						writeTorus->setDefaultData(controller);
						writeTorus->setTransformationModule(mod);
					}

					lineObjects.insert(torusPtr);
				}
			}

			if (!lineObjects.empty()) {

				if (maxPiping)
				{
					for (SafePtr<AGraphNode> newChild : lineObjects)
						AGraphNode::addOwningLink(maxPiping, newChild);
				}
				else {
					SafePtr<ClusterNode> cluster = make_safe<ClusterNode>();
					{
						WritePtr<ClusterNode> wCluster = cluster.get();
						if (wCluster)
						{
							wCluster->setTreeType(TreeType::Piping);
							wCluster->setName(TEXT_DEFAULT_NAME_PIPING.toStdWString());
							wCluster->setDefaultData(controller);
						}
					}

					for (SafePtr<AGraphNode> object : lineObjects)
						AGraphNode::addOwningLink(cluster, object);

					lineObjects.insert(cluster);
				}

				controller.getControlListener()->notifyUIControl(new control::function::AddNodes(lineObjects));
			}
		}
	}

	return validate(controller);

}

bool ContextPipePostConnexion::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextPipePostConnexion::getType() const
{
	return (ContextType::pipePostConnexion);
}

ContextState ContextPipePostConnexion::abort(Controller& controller)
{
	controller.getControlListener()->notifyUIControl(new control::meta::ControlStopMetaControl());
	return AContext::abort(controller);
}

ContextState ContextPipePostConnexion::validate(Controller& controller)
{
	controller.getControlListener()->notifyUIControl(new control::meta::ControlStopMetaControl());
	return AContext::validate(controller);
}
