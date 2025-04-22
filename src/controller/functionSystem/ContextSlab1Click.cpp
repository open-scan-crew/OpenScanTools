#include "controller/functionSystem/ContextSlab1Click.h"
#include "controller/messages/SimpleNumberMessage.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/RayTracingTexts.hpp"
#include "gui/texts/ContextTexts.hpp"
#include "controller/Controller.h"
#include "controller/IControlListener.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "utils/Logger.h"
#include "controller/controls/ControlFunction.h"

#include "models/graph/GraphManager.h"
#include "models/graph/BoxNode.h"

#include <glm/gtx/quaternion.hpp>


ContextSlab1Click::ContextSlab1Click(const ContextId& id)
	: ARayTracingContext(id)
{
	// The context needs 1 click
	m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_RAYTRACING_DEFAULT_1 });
	
}

ContextSlab1Click::~ContextSlab1Click()
{
}

ContextState ContextSlab1Click::start(Controller& controller)
{
	controller.updateInfo(new GuiDataTmpMessage(TEXT_POINTTOCYLINDER_START, 0));
	return ARayTracingContext::start(controller);
}

ContextState ContextSlab1Click::feedMessage(IMessage* message, Controller& controller)
{
	ARayTracingContext::feedMessage(message, controller);
	switch (message->getType())
	{
	case IMessage::MessageType::SIMPLE_NUMBER:
	{
		SimpleNumberMessage* numberMessage = static_cast<SimpleNumberMessage*>(message);
		m_extend = numberMessage->m_returnedValue;
		break;
	}
	default:
		break;
	}
	return m_state;
}

ContextState ContextSlab1Click::launch(Controller& controller)
{
	// --- Ray Tracing ---
	ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
	// -!- Ray Tracing -!-

	m_state = ContextState::running;
	FUNCLOG << "ContextSlabDetection launch" << LOGENDL;

	GraphManager& graphManager = controller.getGraphManager();

	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));

	bool success = false;

	TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));
	ClippingAssembly clippingAssembly;
	graphManager.getClippingAssembly(clippingAssembly, true, false);
	glm::dvec3 boxCenter, boxDirectionX, boxDirectionY, boxDirectionZ;
	glm::dvec3 seedPoint = m_clickResults[0].position;
	
	std::vector<std::vector<double>> planes;
	std::vector<glm::dvec3> centers;
	glm::dvec3 corner;

	glm::dvec3 scale;
	bool extend = (m_extend == 1);
	success = TlScanOverseer::getInstance().fitSlabTest(seedPoint, planes, centers, corner, clippingAssembly, scale, boxDirectionX, boxDirectionY, boxDirectionZ, boxCenter,extend);
	if (!success)
	{
		m_clickResults.clear();
		return waitForNextPoint(controller, TEXT_SLAB_NOT_FOUND);
	}

	SafePtr<BoxNode> box = make_safe<BoxNode>();
	WritePtr<BoxNode> wBox = box.get();
	if (!wBox)
	{
		m_clickResults.clear();
		return waitForNextPoint(controller);
	}
	wBox->setDefaultData(controller);

	double sign1(1.0), sign2(1.0);
	TransformationModule mod;
	glm::dquat quat1 = glm::rotation(glm::dvec3(0.0, 1.0, 0.0), sign1 * boxDirectionX);
	glm::dvec3 up = glm::normalize(glm::cross(sign1 * boxDirectionX, sign2 * boxDirectionY));
	glm::dvec3 newUp = quat1 * glm::dvec3(0.0, 0.0, 1.0);
	glm::dquat quat2 = glm::rotation(newUp, up);
	//glm::dquat quat2 = glm::rotation(newUp, boxDirectionZ);
	Logger::log(LoggerMode::rayTracingLog) << "boxDirectionX : " << boxDirectionX[0] << " " << boxDirectionX[1] << " " << boxDirectionX[2] << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "boxDirectionY : " << boxDirectionY[0] << " " << boxDirectionY[1] << " " << boxDirectionY[2] << Logger::endl;


	double angle1 = acos(glm::dot(glm::dvec3(1.0, 0.0, 0.0), boxDirectionX));
	double angle2 = acos(glm::dot(glm::dvec3(0.0, 1.0, 0.0), boxDirectionY));
	if (abs(angle1 - angle2) > 0.5)
	{
		angle2 = acos(glm::dot(glm::dvec3(0.0, -1.0, 0.0), boxDirectionY));
	}

	double angle = 0.5 * (angle1 + angle2);
	quat2 = glm::angleAxis(angle, glm::dvec3(0.0, 0.0, 1.0));
	mod.setRotation(quat2);
	mod.setPosition(boxCenter);

	mod.setScale(scale);

	wBox->setTransformationModule(mod);

	controller.getControlListener()->notifyUIControl(new control::function::AddNodes(box));

	m_clickResults.clear();
	return waitForNextPoint(controller);
}

bool ContextSlab1Click::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextSlab1Click::getType() const
{
	return (ContextType::Slab1Click);
}
