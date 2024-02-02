#include "controller/functionSystem/ContextPointCreation.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/controls/ControlFunction.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/Texts.hpp"

#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"
#include "models/3d/Graph/PointNode.h"

ContextPointCreation::ContextPointCreation(const ContextId& id)
	: ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_POINT_PICKING });
}

ContextPointCreation::~ContextPointCreation()
{}

ContextState ContextPointCreation::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	controller.updateInfo(new GuiDataTmpMessage(TEXT_POINT_PICKING_DONE));
	SafePtr<PointNode> point = make_safe<PointNode>();
	WritePtr<PointNode> wPoint = point.get();
	if (!wPoint)
	{
		m_clickResults.clear();
		return waitForNextPoint(controller);
	}

	wPoint->setDefaultData(controller);
	wPoint->setPosition(m_clickResults[0].position);

	controller.getControlListener()->notifyUIControl(new control::function::AddNodes({ point }));
	
	m_clickResults.clear();
	return waitForNextPoint(controller);
}

ContextType ContextPointCreation::getType() const
{
	return ContextType::pointCreation;
}

bool ContextPointCreation::canAutoRelaunch() const
{
    return true;
}