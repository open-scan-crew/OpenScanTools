#include "controller/functionSystem/AContext.h"
#include "controller/Controller.h"
#include "gui/GuiData/GuiDataMessages.h"

AContext::AContext(const ContextId& id)
	: m_id(id)
	, m_state(ContextState::waiting_for_input)
{}

ContextState AContext::abort(Controller& controller)
{
	controller.updateInfo(new GuiDataTmpMessage());
	return (m_state = ContextState::abort);
}

ContextState AContext::validate(Controller& controller)
{
	controller.updateInfo(new GuiDataTmpMessage());
	return (m_state = ContextState::done);
}

void AContext::setState(ContextState state)
{
	m_state = state;
}

ContextState AContext::getState() const
{
	return m_state;
}

ContextId AContext::getId() const
{
	return m_id;
}
