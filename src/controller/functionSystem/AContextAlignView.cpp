#include "controller/functionSystem/AContextAlignView.h"
#include "controller/messages/CameraMessage.h"
#include "gui/GuiData/GuiDataContextRequest.h"
#include "controller/Controller.h"
#include "gui/GuiData/GuiDataMessages.h"

#include "gui/texts/AlignViewTexts.hpp"

#include "utils/Logger.h"
#include "magic_enum/magic_enum.hpp"

AContextAlignView::AContextAlignView(const ContextId& id)
	: ARayTracingContext(id)
{}

AContextAlignView::~AContextAlignView()
{}

ContextState AContextAlignView::start(Controller& controller)
{
	controller.updateInfo(new GuiDataContextRequestActiveCamera(m_id));
	return ARayTracingContext::start(controller);
}

ContextState AContextAlignView::feedMessage(IMessage* message, Controller& controller)
{
    ARayTracingContext::feedMessage(message, controller);

	switch (message->getType())
	{
	case  IMessage::MessageType::CAMERA:
	{
		auto out = static_cast<CameraMessage*>(message);
		m_cameraNode = out->m_cameraNode;
		controller.updateInfo(new GuiDataTmpMessage(TEXT_ALIGN_VIEW_START));
		return (m_state = (m_clickResults.size() == m_usages.size()) ? ContextState::ready_for_using : ContextState::waiting_for_input);
	}
	default:
		FUNCLOG << "wrong message type (" << magic_enum::enum_name<IMessage::MessageType>(message->getType())<< ")" << LOGENDL;
		break;
	}
	return (m_state);
}

bool AContextAlignView::canAutoRelaunch() const
{
	return (false);
}