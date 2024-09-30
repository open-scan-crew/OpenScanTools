#include "controller/functionSystem/ContextTemplateListModification.h"
#include "controller/controls/ControlTemplateEdit.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/messages/DataIdListMessage.h"
#include "controller/messages/ModalMessage.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/ContextTexts.hpp"
#include "gui/GuiData/GuiDataTemplate.h"

#include "models/3d/Graph/TagNode.h"
#include "models/3d/Graph/GraphManager.hxx"


#include "utils/Logger.h"
 
// Note (Aurélien) QT::StandardButtons enum values in qmessagebox.h
#define Yes 0x00004000
#define Cancel 0x00400000

ContextTemplateListModification::ContextTemplateListModification(const ContextId& id)
	: AContext(id)
{
}

ContextTemplateListModification::~ContextTemplateListModification()
{}

ContextState  ContextTemplateListModification::start(Controller& controller)
{
	return (m_state = ContextState::waiting_for_input);
}

ContextState  ContextTemplateListModification::feedMessage(IMessage* message, Controller& controller)
{
	switch (message->getType())
	{
	case IMessage::MessageType::TEMPLATE_LIST:
	{
		TemplateListMessage* out = static_cast<TemplateListMessage*>(message);
		m_temp = out->m_temp;
		m_fieldId = out->m_fieldId;
		m_newType = out->m_newType;
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
		FUNCLOG << "ContextTemplateListModification feedMessage size " << out->m_dataPtrs.size() << LOGENDL;
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

ContextState ContextTemplateListModification::launch(Controller& controller)
{
	m_state = ContextState::running;
	FUNCLOG << "ContextTemplateListModification launch" << LOGENDL;

	GraphManager& graphManager = controller.getGraphManager();

	{
		WritePtr<sma::TagTemplate> wTagTemp = m_temp.get();
		if (!wTagTemp || wTagTemp->getFields().find(m_fieldId) == wTagTemp->getFields().end())
			return m_state = ContextState::abort;
		wTagTemp->modifyFieldReference(m_fieldId, m_newType);
	}

	controller.updateInfo(new GuiDataSendTagTemplate(m_temp, controller.getContext().getUserLists()));

	//fixme (Aurélien) rules set for reseting only if needed
	//std::unordered_map<sma::tFieldType, std::unordered_map<sma::tFieldType, bool>> rules;
	//if (rules[controller.getContext().getTemplates().at(_tempId).getFields().at(_fieldId).m_type][_newType])
	//{
		for (const SafePtr<AGraphNode>& tag : m_affectedTags)
		{
			WritePtr<TagNode> writeTag = static_pointer_cast<TagNode>(tag).get();
			if (!writeTag)
				continue;
			time_t timeNow;
			writeTag->removeField(m_fieldId);
			writeTag->setCreationTime(time(&timeNow));
		}
	//}

	FUNCLOG << "ContextTemplateListModification launch end" << LOGENDL;
	/*
	std::unordered_set<SafePtr<AGraphNode>> selected(controller.getGraphManager().getSelectedNodes());
	if (selected.empty() || selected.size() > 1)
		return (m_state = ContextState::done);

	SafePtr<AGraphNode> select = *(selected.begin());
	ReadPtr<AGraphNode> rSelect = select.cget();

	if(!rSelect || rSelect->getType() != ElementType::Tag)
		return (m_state = ContextState::done);

	controller.actualizeOnId(*(selected.begin()), true);
	*/

	return (m_state = ContextState::done);
}

bool ContextTemplateListModification::canAutoRelaunch() const
{
	return (false);
}

ContextType ContextTemplateListModification::getType() const
{
	return (ContextType::templateListModification);
}
