#include "controller/functionSystem/ContextBigCylinderFit.h"
#include "controller/controls/ControlFunctionTag.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "controller/messages/PipeMessage.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "pointCloudEngine/MeasureClass.h"
#include "controller/controls/ControlFunction.h"
#include "utils/Logger.h"

#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"
#include "models/3d/Graph/CylinderNode.h"

#include <glm/gtx/quaternion.hpp>

ContextBigCylinderFit::ContextBigCylinderFit(const ContextId& id)
	: ARayTracingContext(id)
{
	resetClickUsages();
}

ContextBigCylinderFit::~ContextBigCylinderFit()
{
}

void ContextBigCylinderFit::resetClickUsages()
{
	m_usages.clear();
	m_clickResults.clear();
	m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_BIGCYLINDERFIT_START });
	m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_BIGCYLINDERFIT_SECOND });
	m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_BIGCYLINDERFIT_THIRD });
	m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_BIGCYLINDERFIT_FOURTH });
}

ContextState ContextBigCylinderFit::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState ContextBigCylinderFit::feedMessage(IMessage* message, Controller& controller)
{
	return ARayTracingContext::feedMessage(message, controller);
}

ContextState ContextBigCylinderFit::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-


	PipeDetectionOptions options = controller.getContext().getPipeDetectionOptions();

    FUNCLOG << "ContextBigCylinderFit launch" << LOGENDL;
	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));

	OpenScanToolsGraphManager& graphManager = controller.getOpenScanToolsGraphManager();
    ClippingAssembly clippingAssembly;
	graphManager.getClippingAssembly(clippingAssembly, true, false);

	glm::dvec3 cylinderDirection, cylinderCenter;
	double cylinderRadius, radius(0.3);
	//radius = 0.5*glm::length(m_points[0] - m_points[1]);
	std::vector<glm::dvec3> tags;
	std::vector<std::vector<double>> planes;
    TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));

	//previous bigCylinder
	//bool success = TlScanOverseer::getInstance().fitBigCylinder(m_points[0], m_points[1], radius, 0.004, cylinderRadius, cylinderDirection, cylinderCenter, tags, planes,m_clippingBoxes);
	
	//new 4ClicksCylinder
	std::vector<glm::dvec3> seedPoints;
	for (int i = 0; i < 4; i++)
	{
		seedPoints.push_back(m_clickResults[i].position);
	}
	bool success = TlScanOverseer::getInstance().fitCylinder4Clicks(seedPoints, radius, 0.004, cylinderRadius, cylinderDirection, cylinderCenter, clippingAssembly);
	
	//box for planes
	/*const ClippingBoxSettings& settings = controller.getContext().getClippingSettings();
	smp::Clipping* box = new smp::Clipping("Clipping Box", controller.getContext().getCurrentProject()->getNextUserId(ElementType::Box), controller.getContext().getActiveAuthor());
	box->setColor(settings.color);
	glm::dvec3 boxSize(3.0, 3.0, 0.2);
	box->setSize(boxSize);
	glm::dvec3 boxOrientation(0.0, -atan(sqrt(planes[0][0]*planes[0][0]+planes[0][1]*planes[0][1])/planes[0][2]),atan(planes[0][1]/planes[0][0]));

	box->setOrientation(boxOrientation);
	
	box->setCenter(m_clickResults[0]);
	
	box->setVisible(true);
	box->setActive(false);
	time_t timeNow;
	box->setTime(time(&timeNow));
	controller.getControlListener()->notifyUIControl(new control::function::clipping::CreateClippingBox(box));
	//second box//
	/*const ClippingBoxSettings& settings = controller.getContext().getClippingSettings();
	smp::Clipping* box = new smp::Clipping("Clipping Box", controller.getContext().getCurrentProject()->getNextUserId(ElementType::Box), controller.getContext().getActiveAuthor());
	box2->setColor(settings.color);
	box2->setSize(boxSize);
	glm::dvec3 boxOrientation2(0.0, -atan(sqrt(planes[1][0] * planes[1][0] + planes[1][1] * planes[1][1]) / planes[1][2]), atan(planes[1][1] / planes[1][0]));

	box2->setOrientation(boxOrientation2);

	box2->setCenter(m_clickResults[1]);

	box2->setVisible(true);
	box2->setActive(false);
	time_t timeNow2;
	box2->setTime(time(&timeNow2));
	controller.getControlListener()->notifyUIControl(new control::function::clipping::CreateClippingBox(box2));*/
	//end clipping for planes//

	/*for (int i = 0; i < (int)tags.size(); i++)
	{
		Tag *tag = new Tag("tag", controller->getContext().getCurrentProject()->getNextUserId(ElementType::Tag),
			controller->getContext().getCurrentProject()->getProjectInfo()._author,
			controller->getContext().getTemplates().at(controller->getContext().getCurrentTemplate()));
		tag->setName(std::string(""));
		tag->setColor(controller.getContext().getActiveColor());
		tag->setDescription(std::to_string(i));
		tag->setVisible(true);
		tag->setPosition(tags[i]);
		time_t timeNow;
		tag->setTime(time(&timeNow));
		controller.getControlListener()->notifyUIControl(new control::function::tag::CreateTag(tag));
	}*/

	QString abortPipeErrorText = QString();
	if (!success || m_state != ContextState::running)
		abortPipeErrorText = TEXT_CYLINDER_NOT_FOUND;
	else if (options.insulatedThickness >= cylinderRadius)
		abortPipeErrorText = TEXT_CYLINDER_INSULATED_THICKNESS_EXCEEDS;

	if (!abortPipeErrorText.isEmpty())
	{
		if (m_state == ContextState::running)
		{
			m_clickResults.clear();
			return waitForNextPoint(controller, abortPipeErrorText);
		}
		else
			return m_state == ContextState::done ? ARayTracingContext::validate(controller) : ((m_state == ContextState::abort) ? ARayTracingContext::abort(controller) : m_state);
	}

	SafePtr<CylinderNode> cylinderNode = make_safe<CylinderNode>(cylinderRadius);
	WritePtr<CylinderNode> wCyl = cylinderNode.get();

	wCyl->setDefaultData(controller);

	TransformationModule mod;
	mod.setRotation(glm::dquat(glm::rotation(glm::dvec3(0.0, 0.0, 1.0), cylinderDirection)));
	cylinderCenter = 0.5*(MeasureClass::projectPointToLine(m_clickResults[2].position, cylinderDirection, cylinderCenter) + MeasureClass::projectPointToLine(m_clickResults[3].position, cylinderDirection, cylinderCenter));
	mod.setPosition(cylinderCenter);
	wCyl->setTransformationModule(mod);

	double length = glm::length(MeasureClass::projectPointToLine(m_clickResults[2].position, cylinderDirection, cylinderCenter) - MeasureClass::projectPointToLine(m_clickResults[3].position, cylinderDirection, cylinderCenter));
	wCyl->setLength(abs(length));

	controller.getControlListener()->notifyUIControl(new control::function::AddNodes(cylinderNode));
    
	m_clickResults.clear();
	return waitForNextPoint(controller);
}

bool ContextBigCylinderFit::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextBigCylinderFit::getType() const
{
	return (ContextType::bigCylinderFit);
}
