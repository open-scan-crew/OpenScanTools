#include "controller/functionSystem/ContextSphere.h"
#include "controller/messages/FullClickMessage.h"
#include "controller/messages/ClickMessage.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/RayTracingTexts.hpp"
#include "gui/texts/ContextTexts.hpp"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "pointCloudEngine/TlScanOverseer.h"
#include "utils/Logger.h"
#include "controller/controls/ControlFunction.h"
#include "models/3d/Graph/PointNode.h"
#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"
#include "models/3d/Graph/SphereNode.h"

#include "magic_enum/magic_enum.hpp"

ContextSphere::ContextSphere(const ContextId& id)
	: ARayTracingContext(id)
{
    // The context need 1 click
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_RAYTRACING_DEFAULT_1 });
}

ContextSphere::~ContextSphere()
{
}

ContextState ContextSphere::start(Controller& controller)
{
	controller.updateInfo(new GuiDataTmpMessage(TEXT_POINTTOCYLINDER_START, 0));
	return ARayTracingContext::start(controller);
}

ContextState ContextSphere::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextSphere::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	m_state = ContextState::running;
	FUNCLOG << "ContextFitSphere launch" << LOGENDL;

	OpenScanToolsGraphManager& graphManager = controller.getOpenScanToolsGraphManager();

	controller.updateInfo(new GuiDataTmpMessage(TEXT_LUCAS_SEARCH_ONGOING, 0));
	bool success = false;
	
	glm::dvec3 center, centerOfMass;
	double radius, threshold(0.001);
	TlScanOverseer::setWorkingScansTransfo(graphManager.getVisiblePointCloudInstances(m_panoramic, true, true));
	std::vector<glm::dvec3> seedPoints;
	seedPoints.push_back(m_clickResults[0].position);
	ClippingAssembly clippingAssembly;
	graphManager.getClippingAssembly(clippingAssembly, true, false);

	success = TlScanOverseer::getInstance().fitSphere(seedPoints, center, radius, threshold, clippingAssembly, centerOfMass);
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
	{
		m_clickResults.clear();
		return waitForNextPoint(controller, TEXT_SPHERE_FAILED);
	}

	m_clickResults.clear();
	return waitForNextPoint(controller);
}

bool ContextSphere::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextSphere::getType() const
{
	return (ContextType::Sphere);
}
