#include "controller/functionSystem/ABackgroundContext.h"

ABackgroundContext::ABackgroundContext(const ContextId& id, const ContextId& parent)
	: AContext(id)
	, m_parent(parent)
{}

ContextId ABackgroundContext::getParentId() const
{
	return m_parent;
}