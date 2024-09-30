#include "controller/functionSystem/ContextMoveManip.h"
#include "controller/Controller.h"
#include "models/3d/Graph/GraphManager.hxx"
#include "utils/Logger.h"

#include "gui/texts/ContextTexts.hpp"

ContextMoveManip::ContextMoveManip(const ContextId& id)
	: ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, TEXT_MOVE_MANIP_START });
}

ContextMoveManip::~ContextMoveManip()
{}

ContextState ContextMoveManip::start(Controller& controller)
{
    return ARayTracingContext::start(controller);
}

ContextState ContextMoveManip::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextMoveManip::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	FUNCLOG << "ContextMoveTag launch" << LOGENDL; 

	if(m_clickResults.size() != 1)
		ARayTracingContext::abort(controller);

	SafePtr<ManipulatorNode> manipNode = controller.getGraphManager().getManipulatorNode();
	{
		WritePtr<ManipulatorNode> wManipNode = manipNode.get();
		if(wManipNode)
			wManipNode->setTempManipPos(m_clickResults[0].position);
	}

	return ARayTracingContext::validate(controller);
}

bool ContextMoveManip::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextMoveManip::getType() const
{
	return (ContextType::moveManip);
}
