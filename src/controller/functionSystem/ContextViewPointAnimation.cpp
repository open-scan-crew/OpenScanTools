#include "controller/functionSystem/ContextViewPointAnimation.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "controller/Controller.h"
#include "controller/messages/ClickMessage.h"
#include "controller/messages/RenderContextMessage.h"
#include <glm/glm.hpp>

ContextViewPointAnimation::ContextViewPointAnimation(const ContextId& id)
	: AContext(id)
{}

ContextViewPointAnimation::~ContextViewPointAnimation()
{}

ContextState ContextViewPointAnimation::start(Controller& controller)
{
	return (m_state = ContextState::waiting_for_input);
}

ContextState  ContextViewPointAnimation::feedMessage(IMessage* message, Controller& controller)
{
	switch (message->getType())
	{
		case IMessage::MessageType::RENDER_CONTEXT:
		{
			RenderContextMessage* context = static_cast<RenderContextMessage*>(message);
			//m_currentContext.setParameters(context->m_parameters);
			m_state = ContextState::ready_for_using;
		}
		break;
	}
	
	
	return m_state;
}

ContextState ContextViewPointAnimation::launch(Controller& controller)
{
	m_state = ContextState::running;
	//controller.updateInfo(new GuiDataRenderAnimationViewPoint(m_currentContext));
	return (m_state = ContextState::done);
}

bool ContextViewPointAnimation::canAutoRelaunch() const
{
	return (false);
}

ContextType ContextViewPointAnimation::getType() const
{
	return (ContextType::viewPointAnimation);
}
