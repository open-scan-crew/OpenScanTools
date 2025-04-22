#include "controller/functionSystem/ContextPlaneConnexion.h"
#include "controller/Controller.h"
#include "controller/IControlListener.h"
#include "controller/messages/IMessage.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "utils/Logger.h"
#include "magic_enum/magic_enum.hpp"
#include "pointCloudEngine/TlScanOverseer.h"
#include "controller/controls/ControlFunction.h"
#include "models/graph/BoxNode.h"
#include "models/graph/GraphManager.h"

ContextPlaneConnexion::ContextPlaneConnexion(const ContextId& id)
	: ARayTracingContext(id)
	, m_lastPointInd(-1)
{
	m_usages.push_back({ true, { ElementType::Tag, ElementType::Point }, TEXT_POINT_MEASURE_START });
}

ContextPlaneConnexion::~ContextPlaneConnexion()
{}

ContextState ContextPlaneConnexion::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState ContextPlaneConnexion::feedMessage(IMessage* message, Controller& controller)
{
	ARayTracingContext::feedMessage(message, controller);

	switch (message->getType())
	{
	case IMessage::MessageType::UNDO:
		if (m_lastPointInd > 1)
			m_lastPointInd--;
		else if (m_lastPointInd == 1)
			m_lastPointInd = -1;
		break;
	case IMessage::MessageType::REDO:
		if (m_points.size() > m_lastPointInd + 1)
		{
			if (m_lastPointInd == -1)
				m_lastPointInd = 1;
			else
				m_lastPointInd++;
		}
		break;

	default:
		FUNCLOG << "wrong message type (" << magic_enum::enum_name<IMessage::MessageType>(message->getType()) << ")" << LOGENDL;
		break;
	}
	return (m_state);
}

ContextState ContextPlaneConnexion::launch(Controller& controller)
{
	// --- Ray Tracing ---
	ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));
	bool success = false;

	GraphManager& graphManager = controller.getGraphManager();

	TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));
	ClippingAssembly clippingAssembly;
	graphManager.getClippingAssembly(clippingAssembly, true, false);
	std::vector<double> plane;
	int step = (int)m_clickResults.size() - 1;
	RectangularPlane rectPlane = RectangularPlane(glm::dvec3(1.0, 0.0, 0.0), std::vector<glm::dvec3>(0), glm::dvec3(0.0, 0.0, 0.0), PlaneType::tilted);
	//for now, this function uses localPlane
	if (TlScanOverseer::getInstance().fitLocalPlane(clippingAssembly,m_clickResults[step].position, rectPlane))
	{
		//box for planes
		SafePtr<BoxNode> box = make_safe<BoxNode>();
		WritePtr<BoxNode> wBox = box.get();
		if (!wBox)
		{
			m_clickResults.pop_front();
			return waitForNextPoint(controller);
		}

		TransformationModule mod = rectPlane.createTransfo();
		m_rectPlanes.push_back(rectPlane);
		wBox->setDefaultData(controller);

		wBox->setTransformationModule(mod);

		wBox->setVisible(true);
		wBox->setSelected(false);
		wBox->setClippingActive(false);
		controller.getControlListener()->notifyUIControl(new control::function::AddNodes(box));

		m_planes.push_back(box);
	}
	int size = (int)m_planes.size();
	if (size > 1)
	{
		//extend last two planes to connect
		RectangularPlane rect1(m_rectPlanes[size - 2]), rect2(m_rectPlanes[size - 1]);
		OctreeRayTracing::connectPlanes(rect1, rect2);
		m_rectPlanes[size - 2] = rect1;
		m_rectPlanes[size - 1] = rect2;
		WritePtr<BoxNode> wBox1 = m_planes[size - 2].get();
		WritePtr<BoxNode> wBox2 = m_planes[size - 1].get();
		TransformationModule mod = rect1.createTransfo();
		wBox1->setTransformationModule(mod);
		mod = rect2.createTransfo();
		wBox2->setTransformationModule(mod);
	}
	m_clickResults.pop_front();
	return waitForNextPoint(controller);
}

bool ContextPlaneConnexion::canAutoRelaunch() const
{
	return true;
}

ContextType ContextPlaneConnexion::getType() const
{
	return ContextType::planeConnexion;
}
