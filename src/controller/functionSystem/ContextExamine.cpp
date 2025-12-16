#include "controller/functionSystem/ContextExamine.h"
#include "controller/messages/FullClickMessage.h"
#include "gui/GuiData/GuiDataRendering.h"
#include "controller/Controller.h"
#include "utils/Logger.h"

ContextExamine::ContextExamine(const ContextId& id)
        : ARayTracingContext(id)
        , m_target()
{
    m_usages.push_back({ false,
        { ElementType::Point,
          ElementType::Tag,
          ElementType::Scan,
          ElementType::Box,
          ElementType::Sphere,
          ElementType::Cylinder,
          ElementType::Torus,
          ElementType::Piping,
          ElementType::MeshObject },
        "" });
        m_isDisplayingClickTarget = false;
}

ContextExamine::~ContextExamine()
{}

ContextState ContextExamine::feedMessage(IMessage* message, Controller& controller) 
{
    ARayTracingContext::feedMessage(message, controller);
	switch (message->getType())
    {
        case IMessage::MessageType::FULL_CLICK:
        {
        auto msg = static_cast<FullClickMessage*>(message);
        if (!m_usages.empty())
            m_usages[0].getObjectCenter = msg->m_clickInfo.useObjectCenter;
        m_target = msg->m_clickInfo.viewport;
        break;
        }
	default:
		break;
	}
	return (m_state);
}

ContextState ContextExamine::launch(Controller& controller)
{
    // --- Ray Tracing ---
    ARayTracingContext::getNextPosition(controller);
	if (pointMissing())
		return waitForNextPoint(controller);
    // -!- Ray Tracing -!-

	if (std::isnan(m_clickResults[0].position.x) == true)
	{
		FUNCLOG << "picking nan detected" << LOGENDL;
		controller.updateInfo(new GuiDataRenderExamine(m_target, true));
	}
	else
		controller.updateInfo(new GuiDataRenderExamine(m_target, m_clickResults[0].position));

	return ARayTracingContext::validate(controller);
}

ContextState ContextExamine::abort(Controller& controller)
{
	controller.updateInfo(new GuiDataRenderExamine(m_target, false));
	return ARayTracingContext::abort(controller);
}

bool ContextExamine::canAutoRelaunch() const
{
	return false;
}

ContextType ContextExamine::getType() const
{
	return ContextType::examine;
}
