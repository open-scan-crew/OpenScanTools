#include "controller/functionSystem/ContextTemplateModification.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/messages/DataIdListMessage.h"
#include "controller/messages/ModalMessage.h"
#include "controller/messages/TemplateMessage.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiDataTemplate.h"
#include "gui/texts/ContextTexts.hpp"

#include "models/graph/TagNode.h"
#include "models/graph/GraphManager.h"

#include "utils/Logger.h"

// Note (Aurélien) QT::StandardButtons enum values in qmessagebox.h
#define Yes 0x00004000
#define Cancel 0x00400000

ContextTemplateModification::ContextTemplateModification(const ContextId& id)
	: AContext(id)
{
}

ContextTemplateModification::~ContextTemplateModification()
{}

ContextState  ContextTemplateModification::start(Controller& controller)
{
	return (m_state = ContextState::waiting_for_input);
}

ContextState  ContextTemplateModification::feedMessage(IMessage* message, Controller& controller)
{
	switch (message->getType())
	{
	case IMessage::MessageType::TEMPLATE:
	{
		TemplateMessage* out = static_cast<TemplateMessage*>(message);
		m_temp = out->m_temp;
		m_fieldId = out->m_fieldId;
		m_newType = out->m_newType;
		FUNCLOG << "ContextTemplateModification template " << LOGENDL;
		return (m_state = ContextState::waiting_for_input);
	}
	case IMessage::MessageType::DATAID_LIST:
	{
		DataListMessage* out = static_cast<DataListMessage*>(message);
		if (out->m_type != ElementType::Tag)
		{
			FUNCLOG << "Not tag given" << LOGENDL;
			return (m_state);
		}
		FUNCLOG << "ContextTemplateModification feedMessage size " << out->m_dataPtrs.size() << LOGENDL;
		m_affectedTags = out->m_dataPtrs;
		if(m_affectedTags.empty())
		{ 
			return (m_state = ContextState::ready_for_using);
		}
		else
		{
			controller.updateInfo(new GuiDataModal(Yes | Cancel, TEXT_TEMPLATE_MODIFICATION_QUESTION));
			return (m_state = ContextState::waiting_for_input);
		}
	}
	case IMessage::MessageType::MODAL:
	{
		ModalMessage* modal = static_cast<ModalMessage*>(message);
		switch (modal->m_returnedValue)
		{
		case Yes:
			return (m_state = ContextState::ready_for_using);
		case Cancel:
			return (m_state = ContextState::done);
		}
	}
	}
	return (m_state = ContextState::waiting_for_input);
}

ContextState ContextTemplateModification::launch(Controller& controller)
{
	m_state = ContextState::running;
	FUNCLOG << "ContextTemplateModification launch" << LOGENDL;

	GraphManager& graphManager = controller.getGraphManager();

	{
		WritePtr<sma::TagTemplate> wTagTemp = m_temp.get();
		if (!wTagTemp || wTagTemp->getFields().find(m_fieldId) == wTagTemp->getFields().end())
			return (m_state = ContextState::abort);
		wTagTemp->modifyFieldType(m_fieldId, m_newType);
		if (m_newType == sma::tFieldType::list)
			wTagTemp->modifyFieldReference(m_fieldId, *controller.getContext().getUserLists().begin());
	}

	controller.updateInfo(new GuiDataSendTagTemplate(m_temp, controller.getContext().getUserLists()));

	for (const SafePtr<AGraphNode>& dataPtr : m_affectedTags)
	{
		WritePtr<TagNode> tag = static_pointer_cast<TagNode>(dataPtr).get();
		if (!tag)
			continue;
		time_t timeNow;
		tag->removeField(m_fieldId);
		tag->setCreationTime(time(&timeNow));
	}

	FUNCLOG << "ContextTemplateModification launch end" << LOGENDL;

	return (m_state = ContextState::done);
}

bool ContextTemplateModification::canAutoRelaunch() const
{
	return (false);
}

ContextType ContextTemplateModification::getType() const
{
	return (ContextType::templateModification);
}
