#include "controller/functionSystem/ContextCreateTag.h"
#include "controller/controls/ControlFunction.h"

#include "models/graph/TagNode.h"
#include "models/graph/GraphManager.hxx"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/functionSystem/FunctionManager.h"
#include "gui/GuiData/GuiDataGeneralProject.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"

#include "utils/Logger.h"

ContextCreateTag::ContextCreateTag(const ContextId& id)	
	: ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Point, ElementType::Tag}, "" });
}

ContextCreateTag::~ContextCreateTag()
{
}

ContextState ContextCreateTag::start(Controller&  controller)
{
	return ARayTracingContext::start(controller);
}

ContextState  ContextCreateTag::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextCreateTag::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	FUNCLOG << "ContextCreateTag launch" << LOGENDL;
	const std::unordered_set<SafePtr<sma::TagTemplate>>& templates = controller.getContext().cgetTemplates();
	if (templates.find(controller.getContext().getCurrentTemplate()) != templates.end())
	{
		SafePtr<TagNode> tagNode = make_safe<TagNode>();
		WritePtr<TagNode> wTagNode = tagNode.get();
		if (!wTagNode)
		{
			m_clickResults.clear();
			return waitForNextPoint(controller);
		}

		wTagNode->setDefaultData(controller);
		wTagNode->setVisible(true);
		wTagNode->setPosition(m_clickResults[0].position);

		controller.getControlListener()->notifyUIControl(new control::function::AddNodes(tagNode));
	}
	else
		controller.updateInfo(new GuiDataWarning(TEXT_TEMPLATE_NOT_SELECTED));

    m_clickResults.clear();
	return waitForNextPoint(controller);
}

bool ContextCreateTag::canAutoRelaunch() const
{
	return (true);
}

ContextType ContextCreateTag::getType() const
{
	return (ContextType::tagCreation);
}
