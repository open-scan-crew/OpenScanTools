#include "controller/functionSystem/ContextDuplicateTag.h"
#include "controller/controls/ControlFunctionTag.h"
#include "controller/controls/ControlFunction.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h" // forward declaration
#include "utils/Logger.h"
#include "gui/texts/ContextTexts.hpp"

#include "models/graph/GraphManager.hxx"

ContextDuplicateTag::ContextDuplicateTag(const ContextId& id)
	: ARayTracingContext(id)
{
    m_usages.push_back({ true, {ElementType::Tag}, TEXT_INFO_DUPLICATE_TAG_PICK });
}

ContextDuplicateTag::~ContextDuplicateTag()
{}

ContextState ContextDuplicateTag::start(Controller & controller)
{
	ARayTracingContext::start(controller);

	if (!refreshDupId(controller))
		return ARayTracingContext::abort(controller);

	return (m_state = ContextState::waiting_for_input);
}


ContextState  ContextDuplicateTag::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);
    return m_state;
}

ContextState ContextDuplicateTag::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	FUNCLOG << "ContextDuplicateTag launch" << LOGENDL; 
	bool isCreation;
	{
		ReadPtr<AGraphNode> rTargetObject = m_clickResults[0].object.cget();
		isCreation = !rTargetObject || rTargetObject->getType() != ElementType::Tag;
	}

	if (isCreation && std::isnan(m_clickResults[0].position.x))
	{
		FUNCLOG << "ContextDuplicateTag launch picking nan detected" << LOGENDL;
		m_clickResults.clear();
		return waitForNextPoint(controller);
	}

	GraphManager& graphManager = controller.getGraphManager();

	if (isCreation)
	{
		SafePtr<TagNode> createNode;
		{
			ReadPtr<TagNode> readDupNode = static_pointer_cast<TagNode>(m_toDup).cget();
			assert(readDupNode);
			if (!readDupNode)
				return ARayTracingContext::abort(controller);
			createNode = graphManager.createCopyNode(*&readDupNode);
		}
		xg::Guid newId = xg::newGuid();

		WritePtr<TagNode> wCreateNode = createNode.get();

		wCreateNode->setId(newId);
		wCreateNode->setPosition(m_clickResults[0].position);
		time_t timeNow;
		wCreateNode->setCreationTime(time(&timeNow));
		wCreateNode->setModificationTime(time(&timeNow));
		wCreateNode->setAuthor(controller.getContext().getActiveAuthor());
		wCreateNode->setUserIndex(controller.getNextUserId(wCreateNode->getType()));
		wCreateNode->setSelected(false);

		controller.getControlListener()->notifyUIControl(new control::function::AddNodes(createNode));
	}
	else
	{
		SafePtr<sma::TagTemplate> tagTemp;
		{
			ReadPtr<TagNode> readDupNode = static_pointer_cast<TagNode>(m_toDup).cget();
			assert(readDupNode);
			if (!readDupNode)
				return ARayTracingContext::abort(controller);
			tagTemp = readDupNode->getTemplate();
		}

		if (m_clickResults[0].object == m_toDup)
		{
			FUNCLOG << "ContextDuplicateTag Target is origin" << LOGENDL;
			m_clickResults.clear();
			return waitForNextPoint(controller);
		}

		SafePtr<TagNode> destNode = static_pointer_cast<TagNode>(m_clickResults[0].object);
		ReadPtr<TagNode> rDestNode = destNode.cget();
		if (!rDestNode || rDestNode->getTemplate() != tagTemp)
		{
			FUNCLOG << "ContextDuplicateTag not the same template id or safeptr error" << LOGENDL;
			m_clickResults.clear();
			return waitForNextPoint(controller, TEXT_INFO_DUPLICATE_TAG_WRONG_ID);
		}

		controller.getControlListener()->notifyUIControl(new control::function::tag::DuplicateTag(static_pointer_cast<TagNode>(m_toDup), destNode));
	}

	if (!refreshDupId(controller))
		return ARayTracingContext::abort(controller);

	FUNCLOG << "ContextDuplicateTag launch end" << LOGENDL;
	m_clickResults.clear();
	return waitForNextPoint(controller);
}

bool ContextDuplicateTag::canAutoRelaunch() const
{
	return (true);
}

bool ContextDuplicateTag::refreshDupId(Controller& controller)
{
	std::unordered_set<SafePtr<AGraphNode>> selectedTagNodes = controller.getGraphManager().getSelectedNodes();
	if (selectedTagNodes.size() != 1)
		return false;
	SafePtr<AGraphNode> selectObj = (*selectedTagNodes.begin());
	ReadPtr<AGraphNode> readObj = selectObj.cget();
	if (!readObj && readObj->getType() != ElementType::Tag)
		return false;

	m_toDup = selectObj;

	return true;
}

ContextType ContextDuplicateTag::getType() const
{
	return (ContextType::tagDuplication);
}
