#include "controller/functionSystem/ContextDeleteTag.h"
#include "controller/controls/ControlTemplateEdit.h"
#include "controller/controls/ControlSpecial.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/ControlListener.h"
#include "controller/messages/DataIdListMessage.h"
#include "controller/messages/ModalMessage.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/GuiData/GuiData3dObjects.h"
#include "gui/texts/ContextTexts.hpp"

#include "models/graph/TagNode.h"

#include "utils/Logger.h"

// Note (Aurélien) QT::StandardButtons enum values in qmessagebox.h
#define Yes 0x00004000
#define Cancel 0x00400000

ContextDeleteTag::ContextDeleteTag(const ContextId& id)
	: AContext(id)
{
}

ContextDeleteTag::~ContextDeleteTag()
{}

ContextState ContextDeleteTag::start(Controller& controller)
{
	return (m_state = ContextState::waiting_for_input);
}

ContextState ContextDeleteTag::feedMessage(IMessage* message, Controller& controller)
{
	switch (message->getType())
	{
	case IMessage::MessageType::DATAID_LIST:
	{
		DataListMessage* out = static_cast<DataListMessage*>(message);
		if (out->m_type != ElementType::Tag)
		{
			FUNCLOG << "Not tag given" << LOGENDL;
			return (m_state);
		}
		FUNCLOG << "ContextDeleteTag feedMessage size " << out->m_dataPtrs.size() << LOGENDL;
		m_list = out->m_dataPtrs;
		if (m_list.empty())
			return m_state = ContextState::abort;

		controller.updateInfo(new GuiDataModal(Yes | Cancel, TEXT_DELETE_TAGS_QUESTION));
		return (m_state = ContextState::waiting_for_input);
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

ContextState ContextDeleteTag::launch(Controller& controller)
{
	m_state = ContextState::running;
	FUNCLOG << "ContextDeleteTag launch" << LOGENDL;

	SafePtr<TagNode> firstTag = static_pointer_cast<TagNode>(*m_list.begin());
	ReadPtr<TagNode> readTag = firstTag.cget();
	if(!readTag)
		return (m_state = ContextState::abort);

	controller.getContext().getTemplates().erase(readTag->getTemplate());
	readTag->getTemplate().destroy();

	controller.getControlListener()->notifyUIControl(new control::tagTemplate::SendTemplateList());
	controller.getControlListener()->notifyUIControl(new control::special::DeleteElement(m_list, false));

	FUNCLOG << "ContextDeleteTag launch end" << LOGENDL;
	return (m_state = ContextState::done);
}

bool ContextDeleteTag::canAutoRelaunch() const
{
	return (false);
}

ContextType ContextDeleteTag::getType() const
{
	return (ContextType::tagDeletion);
}
