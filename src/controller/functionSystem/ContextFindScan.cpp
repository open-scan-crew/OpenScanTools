#include "controller/functionSystem/ContextFindScan.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"

#include "utils/Logger.h"

ContextFindScan::ContextFindScan(const ContextId& id)
	: ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_POINTTOPLANE_START });
}

ContextFindScan::~ContextFindScan()
{
}

ContextState ContextFindScan::start(Controller&  controller)
{
	return ARayTracingContext::start(controller);
}

ContextState  ContextFindScan::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextFindScan::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	QString scanFound = QString(TEXT_CONTEXT_NO_SCAN_FOUND);
	if (!pointMissing())
		scanFound = QString::fromStdString(m_clickResults[0].scanName);

	controller.updateInfo(new GuiDataInfo(TEXT_CONTEXT_FIND_SCAN_MODAL.arg(scanFound), false));

    m_clickResults.clear();
	return waitForNextPoint(controller);
}

bool ContextFindScan::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextFindScan::getType() const
{
	return (ContextType::findScan);
}
