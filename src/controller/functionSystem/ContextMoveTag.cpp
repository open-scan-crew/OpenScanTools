#include "controller/functionSystem/ContextMoveTag.h"
#include "controller/controls/ControlObject3DEdition.h"
#include "controller/Controller.h"
#include "models/3d/Graph/OpenScanToolsGraphManager.hxx"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "utils/Logger.h"

ContextMoveTag::ContextMoveTag(const ContextId& id)
	: ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, "" });
}

ContextMoveTag::~ContextMoveTag()
{}

ContextState ContextMoveTag::start(Controller& controller)
{
	std::unordered_set<SafePtr<AGraphNode>> selects = controller.getOpenScanToolsGraphManager().getSelectedNodes();
	if (selects.size() != 1)
		return ARayTracingContext::abort(controller);

	ReadPtr<AGraphNode> select = SafePtr<AGraphNode>(*(selects.begin())).cget();
	if(select->getType() != ElementType::Tag)
		return ARayTracingContext::abort(controller);


	m_toMoveData = *(selects.begin());
    return ARayTracingContext::start(controller);
}

ContextState ContextMoveTag::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextMoveTag::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	FUNCLOG << "ContextMoveTag launch" << LOGENDL; 

	controller.getControlListener()->notifyUIControl(new control::object3DEdition::SetCenter(m_toMoveData, m_clickResults[0].position));

	m_clickResults.clear();
	return waitForNextPoint(controller);
}

bool ContextMoveTag::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextMoveTag::getType() const
{
	return (ContextType::tagMove);
}
