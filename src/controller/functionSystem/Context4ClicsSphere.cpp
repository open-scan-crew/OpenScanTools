#include "controller/functionSystem/Context4ClicsSphere.h"

#include "controller/messages/ClickMessage.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/texts/ContextTexts.hpp"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "models/graph/GraphManager.hxx"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "controller/controls/ControlFunctionClipping.h"
#include "controller/controls/ControlFunctionTag.h"
#include "models/graph/SphereNode.h"
#include "controller/controls/ControlFunction.h"
#include "utils/Logger.h"
#include "models/graph/PointNode.h"


Context4ClicsSphere::Context4ClicsSphere(const ContextId& id)
	: ARayTracingContext(id)
{
    // Set the description of the points needed for the processing
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_POINTTOPLANE_START });
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_FIRST });
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_SECOND });
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_PLANE3_THIRD });
}

Context4ClicsSphere::~Context4ClicsSphere()
{
}

ContextState Context4ClicsSphere::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState Context4ClicsSphere::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
	return (m_state);
}

ContextState Context4ClicsSphere::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

    m_state = ContextState::running;
	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));

	GraphManager& graphManager = controller.getGraphManager();

    ClippingAssembly clippingAssembly;
	graphManager.getClippingAssembly(clippingAssembly, true, false);

	bool success = false;
    std::vector<glm::dvec3> seedPoints;
	for (int i = 0; i < 4; i++)
		seedPoints.push_back(m_clickResults[i].position);
	glm::dvec3 center, centerOfMass;
	double radius, threshold(0.003);
	TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));

	success = TlScanOverseer::getInstance().fitSphere(seedPoints, center, radius, threshold, clippingAssembly, centerOfMass);
	QString errorString;
	if (success)
	{
		SafePtr<SphereNode> sphere = make_safe<SphereNode>(radius);
		WritePtr<SphereNode> wSphere = sphere.get();
		if (!wSphere)
		{
			m_clickResults.clear();
			return waitForNextPoint(controller);
		}
		wSphere->setDefaultData(controller);

		wSphere->setVisible(true);
		wSphere->setPosition(center);

		controller.getControlListener()->notifyUIControl(new control::function::AddNodes(sphere));

		//create center
		SafePtr<PointNode> point = make_safe<PointNode>();
		WritePtr<PointNode> wPoint = point.get();
		if (!wPoint)
		{
			m_clickResults.clear();
			return waitForNextPoint(controller);
		}

		wPoint->setDefaultData(controller);
		wPoint->setPosition(center);
		std::wstring name = L"sphere_center";
		wPoint->setName(name);
		controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ point }));
	}
	else
        errorString = TEXT_SPHERE_FAILED;

	m_clickResults.clear();
	return waitForNextPoint(controller);
}


bool Context4ClicsSphere::canAutoRelaunch() const
{
	return (true);
}

ContextType Context4ClicsSphere::getType() const
{
	return (ContextType::ClicsSphere4);
}
