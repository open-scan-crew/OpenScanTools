#include "controller/functionSystem/ContextPlaneDetection.h"
#include "controller/messages/PlaneMessage.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "controller/Controller.h"
#include "controller/IControlListener.h"
#include "models/graph/BoxNode.h"
#include "controller/controls/ControlFunction.h"
#include "models/graph/GraphManager.h"


ContextPlaneDetection::ContextPlaneDetection(const ContextId& id)
	: ARayTracingContext(id)
{
	resetClickUsages();
}

ContextPlaneDetection::~ContextPlaneDetection()
{
}

void ContextPlaneDetection::resetClickUsages()
{
	m_usages.clear();
	m_clickResults.clear();

	if (m_options == PlaneDetectionOptions::multipleSeeds)
	{
		//n
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_FIRST });
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_FIRST });
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_FIRST });

	}
	if (m_options == PlaneDetectionOptions::vertical)
	{
		//need 2 clicks
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_FIRST });
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_SECOND });

	}
	else
	{
		//need 1 click
		m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_POINTTOPLANE_SELECT_PLANE });	
	}
	
}

ContextState ContextPlaneDetection::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState ContextPlaneDetection::feedMessage(IMessage* message, Controller& controller)
{
	ARayTracingContext::feedMessage(message, controller);
	switch (message->getType())
	{
	case IMessage::MessageType::PLANEMESSAGE:
	{
		PlaneMessage* planeMessage = static_cast<PlaneMessage*>(message);
		m_options = planeMessage->getOptions();
		resetClickUsages();
		break;
	}
	default:
		break;
	}
	return (m_state);
}

ContextState ContextPlaneDetection::launch(Controller& controller)
{
	// --- Ray Tracing ---
	ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
	// -!- Ray Tracing -!-
	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));
	bool success = false;

	GraphManager& graphManager = controller.getGraphManager();
	ClippingAssembly clippingAssembly;
	graphManager.getClippingAssembly(clippingAssembly, true, false);

	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));

	TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));
	
	
	if (m_options == PlaneDetectionOptions::autoExtend)
	{
		RectangularPlane rectPlane = RectangularPlane(glm::dvec3(1.0, 0.0, 0.0), std::vector<glm::dvec3>(0), glm::dvec3(0.0, 0.0, 0.0), PlaneType::tilted);

		if (!TlScanOverseer::getInstance().fitPlaneAutoExtend(clippingAssembly, m_clickResults[0].position, rectPlane))
		{
			resetClickUsages();
			return waitForNextPoint(controller);
		}
		TransformationModule mod = rectPlane.createTransfo();
		//box for planes
		SafePtr<BoxNode> box = make_safe<BoxNode>();
		WritePtr<BoxNode> wBox1 = box.get();
		if (!wBox1)
		{
			m_clickResults.pop_front();
			return waitForNextPoint(controller);
		}
		wBox1->setDefaultData(controller);

		wBox1->setTransformationModule(mod);

		wBox1->setVisible(true);
		wBox1->setSelected(false);
		wBox1->setClippingActive(false);
		controller.getControlListener()->notifyUIControl(new control::function::AddNodes(box));

	}

	if (m_options == PlaneDetectionOptions::horizontal)
	{
		//box for planes
		SafePtr<BoxNode> box = make_safe<BoxNode>();
		WritePtr<BoxNode> wBox = box.get();
		if (!wBox)
		{
			m_clickResults.pop_front();
			return waitForNextPoint(controller);
		}
		std::vector<glm::dvec3> corners;
		corners.push_back(m_clickResults[0].position + glm::dvec3(sqrt(2) / 4, sqrt(2) / 4, 0.0));
		corners.push_back(m_clickResults[0].position + glm::dvec3(-sqrt(2) / 4, sqrt(2) / 4, 0.0));
		corners.push_back(m_clickResults[0].position + glm::dvec3(sqrt(2) / 4, -sqrt(2) / 4, 0.0));
		corners.push_back(m_clickResults[0].position + glm::dvec3(-sqrt(2) / 4, -sqrt(2) / 4, 0.0));

		RectangularPlane rectPlane = RectangularPlane(glm::dvec3(0.0, 0.0, 1.0), corners, m_clickResults[0].position, PlaneType::horizontal);
		TransformationModule mod = rectPlane.createTransfo();
		wBox->setDefaultData(controller);

		wBox->setTransformationModule(mod);

		wBox->setVisible(true);
		wBox->setSelected(false);
		wBox->setClippingActive(false);
		controller.getControlListener()->notifyUIControl(new control::function::AddNodes(box));
	}

	if (m_options == PlaneDetectionOptions::localPlane)
	{
		RectangularPlane rectPlane = RectangularPlane(glm::dvec3(1.0, 0.0, 0.0), std::vector<glm::dvec3>(0), glm::dvec3(0.0, 0.0, 0.0), PlaneType::tilted);
		TransformationModule mod;
		if (!TlScanOverseer::getInstance().fitLocalPlane(clippingAssembly, m_clickResults[0].position, rectPlane))
		{
			resetClickUsages();
			return waitForNextPoint(controller);
		}
		mod = rectPlane.createTransfo();
		SafePtr<BoxNode> box = make_safe<BoxNode>();
		WritePtr<BoxNode> wBox = box.get();
		if (!wBox)
		{
			m_clickResults.pop_front();
			return waitForNextPoint(controller);
		}
		wBox->setDefaultData(controller);

		wBox->setTransformationModule(mod);

		wBox->setVisible(true);
		wBox->setSelected(false);
		wBox->setClippingActive(false);
		controller.getControlListener()->notifyUIControl(new control::function::AddNodes(box));
	}

	if (m_options == PlaneDetectionOptions::multipleSeeds)
	{
		if (m_clickResults.size() < 3)
			return waitForNextPoint(controller);

		std::vector<glm::dvec3> seedPoints;
		for (int i = 0; i < (int)m_clickResults.size(); i++)
			seedPoints.push_back(m_clickResults[i].position);
		TransformationModule mod;

		TlScanOverseer::getInstance().fitPlaneMultipleSeeds(seedPoints, mod);

		SafePtr<BoxNode> box = make_safe<BoxNode>();
		WritePtr<BoxNode> wBox = box.get();
		if (!wBox)
		{
			m_clickResults.pop_front();
			return waitForNextPoint(controller);
		}
		wBox->setDefaultData(controller);

		wBox->setTransformationModule(mod);

		wBox->setVisible(true);
		wBox->setSelected(false);
		wBox->setClippingActive(false);
		controller.getControlListener()->notifyUIControl(new control::function::AddNodes(box));
	}

	if (m_options == PlaneDetectionOptions::vertical)
	{
		if (m_clickResults.size() < 2)
			return waitForNextPoint(controller);
		TransformationModule mod;

		TlScanOverseer::getInstance().fitVerticalPlane(m_clickResults[0].position, m_clickResults[1].position, mod);

		SafePtr<BoxNode> box = make_safe<BoxNode>();
		WritePtr<BoxNode> wBox = box.get();
		if (!wBox)
		{
			m_clickResults.pop_front();
			return waitForNextPoint(controller);
		}
		wBox->setDefaultData(controller);

		wBox->setTransformationModule(mod);

		wBox->setVisible(true);
		wBox->setSelected(false);
		wBox->setClippingActive(false);
		controller.getControlListener()->notifyUIControl(new control::function::AddNodes(box));
	}
	resetClickUsages();
	return waitForNextPoint(controller);
}

bool ContextPlaneDetection::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextPlaneDetection::getType() const
{
	return (ContextType::planeDetection);
}
