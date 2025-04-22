#include "controller/functionSystem/ContextBeamDetection.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "controller/Controller.h"
#include "controller/IControlListener.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "controller/controls/ControlFunction.h"
#include "utils/Logger.h"

#include "models/graph/GraphManager.h"
#include "models/graph/BoxNode.h"

#include <glm/gtx/quaternion.hpp>

ContextBeamDetection::ContextBeamDetection(const ContextId& id)
	: ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_FIRST });
	m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_BIGCYLINDERFIT_THIRD });
	m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_BIGCYLINDERFIT_FOURTH });

    //m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_BIGCYLINDERFIT_FOURTH });
    m_repeatInput = false;
}

ContextBeamDetection::~ContextBeamDetection()
{
}

ContextState ContextBeamDetection::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState ContextBeamDetection::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return (m_state);
}

ContextState ContextBeamDetection::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	GraphManager& graphManager = controller.getGraphManager();

	std::vector<std::vector<double>> directionRange;
	double beamHeight;
	glm::dvec3 normalVector, beamDirection, orthoDir;
    TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));
	if ((int)m_clickResults.size() != 3)
		return waitForNextPoint(controller);
	glm::dvec3 lastClickResult = m_clickResults[0].position;
	glm::dvec3 beamStart(m_clickResults[1].position), beamEnd(m_clickResults[2].position);
	ClippingAssembly clippingAssembly;
	graphManager.getClippingAssembly(clippingAssembly, true, false);

	if (TlScanOverseer::getInstance().beamDetectionManualExtend(lastClickResult, beamStart, beamEnd, clippingAssembly, normalVector, beamHeight, directionRange, m_lastCameraPos, beamDirection, orthoDir))
	{
		controller.updateInfo(new GuiDataTmpMessage("done"));
		if (m_state != ContextState::running)
			return m_state;
	}
	else
	{
		controller.updateInfo(new GuiDataTmpMessage("failed"));
		if (m_state == ContextState::running)
		return (m_state = (m_state == ContextState::running ? ContextState::waiting_for_input : m_state));
	}
	/*
	//3 box test
	//box for planes
	SafePtr<BoxNode> box1 = make_safe<BoxNode>();

	WritePtr<BoxNode> wBox1 = box1.get();
	if (!wBox1)
	{
		m_clickResults.clear();
		return waitForNextPoint(controller);
	}

	wBox1->setDefaultData(controller);

	TransformationModule mod;
	mod.setRotation(glm::dquat(glm::rotation(glm::dvec3(0.0, 0.0, 1.0), normalVector)));
	mod.setScale(glm::dvec3(0.5*(directionRange[1][1] - directionRange[1][0]),0.5*( directionRange[1][1] - directionRange[1][0]), 0.001));
	mod.setPosition(lastClickResult + beamDirection * 0.1*(directionRange[0][1] - directionRange[0][0]) + 0.5*orthoDir*(directionRange[1][0] + directionRange[1][1]));

	wBox1->setTransformationModule(mod);
	
	
	wBox1->setVisible(true);
	wBox1->setSelected(false);
	wBox1->setClippingActive(false);

	//controller.getControlListener()->notifyUIControl(new control::function::clipping::CreateClippingBox(box1));

	//second box for first plane

	SafePtr<BoxNode> box11 = make_safe<BoxNode>();

	WritePtr<BoxNode> wBox11 = box11.get();
	if (!wBox11)
	{
		m_clickResults.clear();
		return waitForNextPoint(controller);
	}

	wBox11->setDefaultData(controller);

	mod.setRotation(glm::dquat(glm::rotation(glm::dvec3(0.0, 0.0, 1.0), normalVector)));
	mod.setPosition(lastClickResult + beamDirection * 0.1*(directionRange[0][0] - directionRange[0][1]) + 0.5*orthoDir*(directionRange[1][0] + directionRange[1][1]));
	mod.setScale(glm::dvec3(0.5*(directionRange[1][1] - directionRange[1][0]), 0.5*(directionRange[1][1] - directionRange[1][0]), 0.001));

	wBox11->setTransformationModule(mod);


	wBox11->setVisible(true);
	wBox11->setSelected(false);
	wBox11->setClippingActive(false);

	//controller.getControlListener()->notifyUIControl(new control::function::clipping::CreateClippingBox(box11));

	//second box//
	SafePtr<BoxNode> box2 = make_safe<BoxNode>();

	WritePtr<BoxNode> wBox2 = box2.get();
	if (!wBox2)
	{
		m_clickResults.clear();
		return waitForNextPoint(controller);
	}
	
	wBox2->setDefaultData(controller);

	mod.setPosition(lastClickResult + normalVector * beamHeight +0.5*orthoDir*(directionRange[1][0] + directionRange[1][1]));
	mod.setScale(glm::dvec3(0.5*(directionRange[1][1] - directionRange[1][0]), 0.5*(directionRange[1][1] - directionRange[1][0]), 0.001));
	wBox2->setTransformationModule(mod);


	wBox2->setVisible(true);
	wBox2->setSelected(false);
	wBox2->setClippingActive(false);
	wBox2->setClippingMode(controller.getContext().getDefaultClippingMode());

	wBox2->setDescription(L"height : " + std::to_wstring(1000 * beamHeight));
	controller.getControlListener()->notifyUIControl(new control::function::clipping::CreateClippingBox(box2));

	*/

	double beamLength = abs(glm::dot(beamEnd - beamStart, beamDirection));
	double beamDisplacement = 0.5*glm::dot(beamStart + beamEnd - lastClickResult - lastClickResult, beamDirection);
	SafePtr<BoxNode> box1 = make_safe<BoxNode>();
	WritePtr<BoxNode> wBox1 = box1.get();
	if (!wBox1)
	{
		m_clickResults.clear();
		return waitForNextPoint(controller);
	}

	wBox1->setDefaultData(controller);

	TransformationModule mod;

	//
	glm::dquat quat1 = glm::rotation(glm::dvec3(0.0, 1.0, 0.0), beamDirection);
	glm::dvec3 up = glm::normalize(glm::cross(beamDirection, orthoDir));
	glm::dvec3 newUp = quat1 * glm::dvec3(0.0, 0.0, 1.0);
	glm::dquat quat2 = glm::rotation(newUp, up);
	//glm::dquat quat2 = glm::rotation(newUp, boxDirectionZ);
	Logger::log(LoggerMode::rayTracingLog) << "beamDirection : " << beamDirection[0] << " " << beamDirection[1] << " " << beamDirection[2] << Logger::endl;
	Logger::log(LoggerMode::rayTracingLog) << "orthoDir : " << orthoDir[0] << " " << orthoDir[1] << " " << orthoDir[2] << Logger::endl;


	double angle1 = acos(glm::dot(glm::dvec3(1.0, 0.0, 0.0), beamDirection));
	double angle2 = acos(glm::dot(glm::dvec3(0.0, 1.0, 0.0), orthoDir));
	if (abs(angle1 - angle2) > 0.5)
	{
		angle2 = acos(glm::dot(glm::dvec3(0.0, -1.0, 0.0), orthoDir));
	}

	double angle = 0.5 * (angle1 + angle2);
	quat2 = glm::angleAxis(angle, up);
	if (glm::dot(glm::cross(beamDirection, orthoDir), normalVector) < 0)
		orthoDir = -orthoDir;
	glm::dquat q3 = TlScanOverseer::getInstance().computeRotation(beamDirection, orthoDir);
	mod.setRotation(q3);
	//rotation incorrecte
	//mod.setRotation(glm::dquat(glm::rotation(glm::dvec3(0.0, 0.0, 1.0), normalVector)));


	mod.setScale(glm::dvec3(0.5 * beamLength, 0.5 * (directionRange[1][1] - directionRange[1][0]), 0.5*beamHeight));
	mod.setPosition(lastClickResult + 0.5 * orthoDir * (directionRange[1][0] + directionRange[1][1])+beamDirection*beamDisplacement+0.5*normalVector*beamHeight);

	wBox1->setTransformationModule(mod);

	controller.getControlListener()->notifyUIControl(new control::function::AddNodes(box1));

	m_clickResults.clear();
	return waitForNextPoint(controller);
}

bool ContextBeamDetection::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextBeamDetection::getType() const
{
	return (ContextType::beamDetection);
}
