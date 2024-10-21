#include "controller/functionSystem/ContextPipeDetectionConnexion.h"
#include "controller/controls/ControlFunction.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h" // forward declaration
#include "pointCloudEngine/TlScanOverseer.h"
#include "models/graph/CylinderNode.h"
#include "models/graph/TorusNode.h"
#include "models/graph/ClusterNode.h"
#include "utils/Logger.h"
#include "controller/messages/DataIDListMessage.h"
#include "controller/messages/PipeMessage.h"
#include "controller/controls/ControlSpecial.h"
#include "controller/controls/ControlMetaControl.h"

#include "models/graph/GraphManager.h"

#include <glm/gtx/quaternion.hpp>

ContextPipeDetectionConnexion::ContextPipeDetectionConnexion(const ContextId& id)
	: ARayTracingContext(id)
{
	m_repeatInput = false;
}

ContextPipeDetectionConnexion::~ContextPipeDetectionConnexion()
{
}

void ContextPipeDetectionConnexion::resetClickUsages(Controller& controller)
{
	m_options = controller.getContext().getPipeDetectionOptions();

    m_usages.clear();
    m_clickResults.clear();

    if (m_options.extendMode == PipeDetectionExtendMode::Manual)
    {
        m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_BIGCYLINDERFIT_THIRD });
        m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_BIGCYLINDERFIT_FOURTH });
    }
    else
    {
        m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_POINTTOCYLINDER_START });
    }
}

ContextState ContextPipeDetectionConnexion::start(Controller& controller)
{
	resetClickUsages(controller);
	controller.getControlListener()->notifyUIControl(new control::meta::ControlStartMetaControl());
	return ARayTracingContext::start(controller);
}

ContextState ContextPipeDetectionConnexion::feedMessage(IMessage* message, Controller& controller)
{
	ARayTracingContext::feedMessage(message, controller);
	switch (message->getType())
	{
	case IMessage::MessageType::PIPECONNECTIONMESSAGE:
	{
		PipeConnetionMessage* pipeConnectionMessage = static_cast<PipeConnetionMessage*>(message);
		m_RonDext = pipeConnectionMessage->getRonDext();
		m_angleConstraints = pipeConnectionMessage->getAngleConstraints();
		m_keepDiameter = pipeConnectionMessage->getKeepDiameter();
		break;
	}
	case IMessage::MessageType::DATAID_LIST:
	{
		DataListMessage* idatalist = static_cast<DataListMessage*>(message);
		//m_GuidModifiedCylinder.push_back(*idatalist->m_ids.begin());
		break;
	}
	default:
		break;
	}
	return (m_state);
}

ContextState ContextPipeDetectionConnexion::launch(Controller& controller)
{
	Logger::log(LoggerMode::DataLog) << "pipeDetectionConnexion launch" << Logger::endl;

	// --- Ray Tracing ---
	ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
	// -!- Ray Tracing -!-

	GraphManager& graphManager = controller.getGraphManager();

	bool isAutoExtend = m_options.extendMode == PipeDetectionExtendMode::Auto;
	bool isManualExtend = m_options.extendMode == PipeDetectionExtendMode::Manual;

	if (( !isManualExtend || (m_clickResults.size() % 2 == 0)) && (m_clickResults.size() > 0))
	{
		//detect last cylinder
		m_state = ContextState::running;
		controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));
		bool success = false;
		ClippingAssembly clippingAssembly;
		GraphManager& graphManager = controller.getGraphManager();

		graphManager.getClippingAssembly(clippingAssembly, true, false);
		glm::dvec3 cylinderDirection, cylinderCenter;
		double cylinderRadius,standardCylRadius, radius(0.4);
		std::vector<double> heights(2);
		TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));

		double threshold = m_options.noisy ? 0.004 : 0.002;
		FitCylinderMode mode = m_options.optimized ? FitCylinderMode::robust : FitCylinderMode::fast;
		glm::dvec3 lastClickResult = m_clickResults.back().position;

		if (isAutoExtend)
		{
			assert(m_clickResults.size());
			success = TlScanOverseer::getInstance().extendCylinder(lastClickResult, radius, threshold, cylinderRadius, cylinderDirection, cylinderCenter, heights, mode, clippingAssembly);
		}
		else if (isManualExtend) {
			assert(m_clickResults.size() >= 2);
			std::vector<glm::dvec3> seedPoints;
			int j = (int)m_clickResults.size() - 2;
			seedPoints.push_back(m_clickResults[j].position);
			seedPoints.push_back(m_clickResults[j + 1].position);
			success = TlScanOverseer::getInstance().fitCylinderMultipleSeeds(seedPoints, radius, threshold, cylinderRadius, cylinderDirection, cylinderCenter, mode, clippingAssembly);
			heights[0] = TlScanOverseer::computeHeight(m_clickResults[j].position, cylinderDirection, cylinderCenter);
			heights[1] = TlScanOverseer::computeHeight(m_clickResults[j + 1].position, cylinderDirection, cylinderCenter);
			cylinderCenter = cylinderCenter + 0.5*(heights[0] + heights[1])*cylinderDirection;
			if (!success)
			{
				seedPoints[0] = (0.1*m_clickResults[j].position + 0.9*m_clickResults[j + 1].position);
				seedPoints[1] = (0.9*m_clickResults[j].position + 0.1*m_clickResults[j + 1].position);
				success = TlScanOverseer::getInstance().fitCylinderMultipleSeeds(seedPoints, radius, threshold, cylinderRadius, cylinderDirection, cylinderCenter, mode, clippingAssembly);
				heights[0] = TlScanOverseer::computeHeight(m_clickResults[j].position, cylinderDirection, cylinderCenter);
				heights[1] = TlScanOverseer::computeHeight(m_clickResults[j + 1].position, cylinderDirection, cylinderCenter);
				cylinderCenter = cylinderCenter + 0.5*(heights[0] + heights[1])*cylinderDirection;
			}
			if (!success)
			{
				seedPoints[0] = (0.25*m_clickResults[j].position + 0.75*m_clickResults[j + 1].position);
				seedPoints[1] = (0.75*m_clickResults[j].position + 0.25*m_clickResults[j + 1].position);
				success = TlScanOverseer::getInstance().fitCylinderMultipleSeeds(seedPoints, radius, threshold, cylinderRadius, cylinderDirection, cylinderCenter, mode, clippingAssembly);
				heights[0] = TlScanOverseer::computeHeight(m_clickResults[j].position, cylinderDirection, cylinderCenter);
				heights[1] = TlScanOverseer::computeHeight(m_clickResults[j + 1].position, cylinderDirection, cylinderCenter);
				cylinderCenter = cylinderCenter + 0.5*(heights[0] + heights[1])*cylinderDirection;
			}
		}
		else
		{
			assert(m_clickResults.size());
			success = TlScanOverseer::getInstance().fitCylinder(lastClickResult, radius, threshold, cylinderRadius, cylinderDirection, cylinderCenter, mode, clippingAssembly);
			
			cylinderCenter = cylinderCenter + cylinderDirection * glm::dot(cylinderDirection, lastClickResult - cylinderCenter);
		}
		
		QString abortPipeErrorText = QString();
		// Check fitting success and send message to GUI
		if (!success || m_state != ContextState::running)
			abortPipeErrorText = TEXT_CYLINDER_NOT_FOUND;
		else if (m_options.insulatedThickness >= cylinderRadius)
			abortPipeErrorText = TEXT_CYLINDER_INSULATED_THICKNESS_EXCEEDS;

		if (!abortPipeErrorText.isEmpty())
		{
			if (m_state == ContextState::running)
			{
				resetClickUsages(controller);
				return waitForNextPoint(controller, abortPipeErrorText);
			}
			else
				return m_state == ContextState::done ? ARayTracingContext::validate(controller) : ((m_state == ContextState::abort) ? ARayTracingContext::abort(controller) : m_state);
		}

		SafePtr<StandardList> currentStandard = controller.getContext().getCurrentStandard(StandardType::Pipe);
		//get standard radius
		{
			ReadPtr<StandardList> rStand = currentStandard.cget();
			if (rStand)
			{
				const double& detectedDiameter = cylinderRadius * 2.0;
				double standardDiameter(detectedDiameter), difference(std::numeric_limits<double>::max());
				for (const double& value : rStand->clist())
				{
					double diff(abs(detectedDiameter - value));
					if (difference > diff)
					{
						standardDiameter = value;
						difference = diff;
					}
				}
				standardCylRadius = standardDiameter * 0.5;
			}
			else
			{
				standardCylRadius = cylinderRadius;
			}
		}

		if (m_keepDiameter && (m_cylinderRadii.size() > 0))
		{
			cylinderRadius = m_cylinderRadii[0];
			standardCylRadius = m_standardRadii[0];
		}
		m_cylinderCenters.push_back(cylinderCenter);
		m_cylinderDirections.push_back(cylinderDirection);
		if (isAutoExtend || isManualExtend) {
			m_cylinderLengths.push_back(abs(heights[1] - heights[0]));
			m_cylinderLengthsModif.push_back(abs(heights[1] - heights[0]));
		}
		else
		{
			double length;
			if (cylinderRadius < 0.015)
				length = 3 * cylinderRadius;
			else if (cylinderRadius > 0.075)
				length = cylinderRadius;
			else 
				length = cylinderRadius * (2 * (0.015 - cylinderRadius) / 0.06 + 3);
			m_cylinderLengths.push_back(abs(length));
			m_cylinderLengthsModif.push_back(abs(length));
		}

		m_cylinderRadii.push_back(cylinderRadius);
		m_standardRadii.push_back(standardCylRadius);
		m_standards.push_back(currentStandard);
		m_cylinderCentersModif.push_back(cylinderCenter);
		m_cylinderDirectionsModif.push_back(cylinderDirection);

		std::vector<std::vector<glm::dvec3>> elbowEdges;
		
		int size = (int)m_cylinderCenters.size();
		if (size < 2)
		{
			SafePtr<CylinderNode> cylinderPtr = make_safe<CylinderNode>(cylinderRadius);
			WritePtr<CylinderNode> writeCylinder = cylinderPtr.get();
			if (writeCylinder)
			{
				TransformationModule mod;
				mod.setPosition(cylinderCenter);

				mod.setRotation(glm::dquat(glm::rotation(glm::dvec3(0.0, 0.0, 1.0), cylinderDirection)));
				writeCylinder->setDefaultData(controller);
				writeCylinder->setTransformationModule(mod);


				writeCylinder->setLength(m_cylinderLengths[0]);

				time_t timeNow;
				writeCylinder->setCreationTime(time(&timeNow));

				controller.getControlListener()->notifyUIControl(new control::function::AddNodes(cylinderPtr));

				m_cylinders.push_back(cylinderPtr);
			}
		}
		else {
			//this sould call all previous cylinders, not the last two
			std::vector<glm::dvec3> elbowPoints,elbowCenters;
			std::vector<double>angleModifs;
			std::vector<LineConnectionType> connectionType;
			/*centers.push_back(m_cylinderCentersModif[size - 2]);
			centers.push_back(m_cylinderCentersModif[size - 1]);
			dir.push_back(m_cylinderDirectionsModif[size - 2]);
			dir.push_back(m_cylinderDirectionsModif[size - 1]);
			lengths.push_back(m_cylinderLengthsModif[size - 2]);
			lengths.push_back(m_cylinderLengthsModif[size - 1]);
			radii.push_back(m_standardRadii[size - 2]);
			radii.push_back(m_standardRadii[size - 1]);*/

			//bool globalSuccess = TlScanOverseer::getInstance().applyConstraints(centers, dir, lengths, angleModifs, elbowPoints, connectionType, m_RonDext, radii, elbowEdges, m_angleConstraints,elbowCenters);
			bool globalSuccess = TlScanOverseer::getInstance().applyConstraints(m_cylinderCentersModif, m_cylinderDirectionsModif, m_cylinderLengthsModif, angleModifs, elbowPoints, connectionType, m_RonDext, m_standardRadii, elbowEdges, m_angleConstraints, elbowCenters);
			m_connexionType = connectionType;

			if (connectionType[size - 2] == LineConnectionType::elbow)
			{
				m_isElbow.push_back(true);
				//create temp torus
				double mainAngle(acos(-glm::dot(m_cylinderDirectionsModif[size - 2], m_cylinderDirectionsModif[size - 1])));
				double tubeRadius(std::max(m_standardRadii[size - 2], m_standardRadii[size - 1]));
				double detectedAngle(acos(abs(glm::dot(m_cylinderDirections[size - 2], m_cylinderDirections[size - 1]))));
				double mainRadius(m_RonDext * tubeRadius * 2);

				TransformationModule mod2;
				//mod2.setPosition(elbowPoints[size-2]);
				mod2.setPosition(elbowCenters[size - 2]);
				mod2.setScale(glm::vec3(mainRadius + (tubeRadius - m_options.insulatedThickness)));
				//mod2.setScale(glm::dvec3(0.5*(mainRadius + tubeRadius), 0.5*(mainRadius + tubeRadius), tubeRadius));
				double sign1, sign2;
				if (glm::dot(m_cylinderDirectionsModif[size - 2], m_cylinderCentersModif[size - 2] - elbowEdges[size - 2][0]) > 0)
				{
					sign1 = -1;
				}
				else { sign1 = +1; }
				if (glm::dot(m_cylinderDirectionsModif[size - 1], m_cylinderCentersModif[size - 1] - elbowEdges[size - 2][1]) > 0)
				{
					sign2 = 1;
				}
				else { sign2 = -1; }


				glm::dquat quat1 = glm::rotation(glm::dvec3(0.0, 1.0, 0.0), sign1 * m_cylinderDirectionsModif[size - 2]);
				glm::dvec3 up = glm::normalize(glm::cross(sign1 * m_cylinderDirectionsModif[size - 2], sign2 * m_cylinderDirectionsModif[size - 1]));
				glm::dvec3 newUp = quat1 * glm::dvec3(0.0, 0.0, 1.0);
				glm::dquat quat2 = glm::rotation(newUp, up);
				mod2.setRotation(quat2 * quat1);

				SafePtr<TorusNode> torusPtr = make_safe<TorusNode>(mainAngle, mainRadius, tubeRadius, m_options.insulatedThickness);
				WritePtr<TorusNode> writeTorus = torusPtr.get();
				if (writeTorus)
				{
					writeTorus->setDefaultData(controller);
					writeTorus->setTransformationModule(mod2);

					controller.getControlListener()->notifyUIControl(new control::function::AddNodes(torusPtr));

					m_tempElbows.push_back(torusPtr);
					m_elbowIndex.push_back((int)m_tempElbows.size() - 1);
				}
			}
			else
			{
				m_isElbow.push_back(false);
				m_elbowIndex.push_back(0);
			}
			
			TransformationModule mod;
			mod.setPosition(m_cylinderCentersModif[size - 1]);
			mod.setRotation(glm::dquat(glm::rotation(glm::dvec3(0.0, 0.0, 1.0), m_cylinderDirectionsModif[size - 1])));
			//create temp pipe

			SafePtr<CylinderNode> cylinderPtr = make_safe<CylinderNode>(m_cylinderRadii[size - 1]);
			{
				WritePtr<CylinderNode> writeCylinder = cylinderPtr.get();

				if (writeCylinder)
				{
					writeCylinder->setDefaultData(controller);
					writeCylinder->setTransformationModule(mod);
					writeCylinder->setStandard(m_standards[size - 1]);

					//temp check radii
					for (int j = 0; j < (int)m_cylinderRadii.size(); j++)
						Logger::log(LoggerMode::rayTracingLog) << "radii " << m_cylinderRadii[j] << Logger::endl;

					// Set pipe standard
					if (controller.getContext().getCurrentStandard(StandardType::Pipe))
						writeCylinder->setStandard(controller.getContext().getCurrentStandard(StandardType::Pipe));

					writeCylinder->setLength(m_cylinderLengthsModif[size - 1]);

					time_t timeNow;
					writeCylinder->setCreationTime(time(&timeNow));
					writeCylinder->setDescription(L"angle modification : " + std::to_wstring(acos(angleModifs[size - 1]) * 180 / 3.14159));
				}
			}

			controller.getControlListener()->notifyUIControl(new control::function::AddNodes(cylinderPtr));
			
			m_cylinders.push_back(cylinderPtr);
			
			//add : check if there was a modified cylinder, update parameters accordingly

			//update previous cylinder
			{
				WritePtr<CylinderNode> previousCylinder = m_cylinders[m_cylinders.size() - 2].get();
				if (previousCylinder)
				{
					TransformationModule mod1 = previousCylinder->getTransformationModule();
					mod1.setPosition(m_cylinderCentersModif[size - 2]);
					mod1.setRotation(glm::dquat(glm::rotation(glm::dvec3(0.0, 0.0, 1.0), m_cylinderDirectionsModif[size - 2])));
					previousCylinder->setTransformationModule(mod1);
					//previousCylinder->setRadius(m_cylinderRadii[size-2]);

					previousCylinder->setDescription(L"angle modification : " + std::to_wstring(acos(angleModifs[size - 2]) * 180 / 3.14159));

					previousCylinder->setLength(m_cylinderLengthsModif[size - 2]);
				}
			}

			//check if should remove an elbow
			//should add : there could be 2 elbows to remove, and check if elbow should actually be removed
			for (int i = 0; i < (int)m_isElbow.size(); i++)
			{
				if ((m_isElbow[i]) && (!(connectionType[i] == LineConnectionType::elbow)))
				{
					//remove elbow
					m_isElbow[i] = false;
					std::unordered_set<SafePtr<AGraphNode>> tempObjects;
					SafePtr<TorusNode> torus = m_tempElbows[m_elbowIndex[i]];
					ReadPtr<TorusNode> readTor = torus.cget();

					if (readTor)
					{
						tempObjects.insert(torus);
						m_dataToIgnore.push_back(torus);
					}

					controller.getControlListener()->notifyUIControl(new control::special::DeleteElement(tempObjects, true));

				}
				if (connectionType[i] == LineConnectionType::stitching)
				{
					//update corresponding cylinder
					{
						SafePtr<CylinderNode> currentCylinder = m_cylinders[i];
					WritePtr<CylinderNode> writeCylinder = currentCylinder.get();
					if (writeCylinder)
					{
						writeCylinder->setPosition(m_cylinderCentersModif[i]);
						writeCylinder->setRotation(glm::dquat(glm::rotation(glm::dvec3(0.0, 0.0, 1.0), m_cylinderDirectionsModif[i])));
						writeCylinder->setLength(m_cylinderLengthsModif[i]);

						writeCylinder->setDescription(L"angle modification : " + std::to_wstring(acos(angleModifs[i]) * 180 / 3.14159));
					}
					}
				}
			}
		
		}
	}

	resetClickUsages(controller);
	return waitForNextPoint(controller);
}

ContextState ContextPipeDetectionConnexion::abort(Controller& controller)
{
	FUNCLOG << "ContextFitCylinder abort" << LOGENDL;
	return finishLine(controller);
}

ContextState ContextPipeDetectionConnexion::validate(Controller& controller)
{
	return finishLine(controller);
}

ContextState ContextPipeDetectionConnexion::finishLine(Controller& controller)
{
	if (m_cylinders.size() <= 1)
	{
		controller.getControlListener()->notifyUIControl(new control::meta::ControlStopMetaControl());
		return ARayTracingContext::abort(controller);
	}

	m_state = ContextState::running;
	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING));
	bool success = false;

	std::unordered_set<xg::Guid> tempObjects;
	std::vector<glm::dvec3>centers(m_cylinderCenters), directions(m_cylinderDirections), elbowPoints;
	std::vector<double> radii(m_standardRadii), lengths(m_cylinderLengths);

	std::vector<std::vector<glm::dvec3>> elbowEdges;
	std::vector<glm::dvec3> elbowCenters;
	std::vector<double> angleModifs;
	std::vector<LineConnectionType> connectionType;
	std::vector<std::vector<int>> isStitchedTo;
	bool globalSuccess = TlScanOverseer::getInstance().applyConstraints(centers, directions, lengths, angleModifs, elbowPoints, connectionType, m_RonDext, radii, elbowEdges, m_angleConstraints, elbowCenters);
	std::unordered_set<SafePtr<AGraphNode>> lineObjects;


	if (!globalSuccess)
	{
		FUNCLOG << "ContextFitCylinder abort" << LOGENDL;
		controller.getControlListener()->notifyUIControl(new control::meta::ControlStopMetaControl());
		return ARayTracingContext::abort(controller);
	}
	else
	{
		for (int i = 0; i < (int)centers.size(); i++)
		{
			lineObjects.insert(m_cylinders[i]);

			if (i < (int)centers.size() - 1)
			{
				if ((!(connectionType[i] == LineConnectionType::elbow))||(!(m_connexionType[i]==LineConnectionType::elbow)))
				{
					continue;
				}
				
				lineObjects.insert(m_tempElbows[m_elbowIndex[i]]);
			}

		}

		if (!lineObjects.empty()) {
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


			controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ cluster }));
		}
	}

	return ARayTracingContext::validate(controller);
}

bool ContextPipeDetectionConnexion::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextPipeDetectionConnexion::getType() const
{
	return (ContextType::pipeDetectionConnexion);
}
