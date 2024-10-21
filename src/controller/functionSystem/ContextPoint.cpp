#include "controller/functionSystem/ContextPoint.h"
#include "controller/Controller.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataMeasure.h"
#include "gui/Texts.hpp"

ContextPoint::ContextPoint(const ContextId& id)
	: ARayTracingContext(id)
{
    m_usages.push_back({ true, { ElementType::Tag, ElementType::Point, ElementType::ViewPoint }, TEXT_POINT_PICKING });
}

ContextPoint::~ContextPoint()
{}

ContextState ContextPoint::start(Controller& controller)
{
	return ARayTracingContext::start(controller);
}

ContextState ContextPoint::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextPoint::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
    if (pointMissing())
        return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	controller.updateInfo(new GuiDataTmpMessage(TEXT_POINT_PICKING_DONE));
	//Information de création du point
    controller.updateInfo(new GuiDataPoint(m_clickResults[0].position));
    return ARayTracingContext::validate(controller);
}

bool ContextPoint::canAutoRelaunch() const
{
	return true;
}

ContextType ContextPoint::getType() const
{
	return ContextType::pointMeasure;
}
