#ifndef ABACKGROUND_CONTEXT_H
#define ABACKGROUND_CONTEXT_H

#include "controller/functionSystem/AContext.h"

class ABackgroundContext : public AContext
{
public:
	ABackgroundContext(const ContextId& id, const ContextId& parent);
	ContextId getParentId() const;

protected:
	const ContextId m_parent;
};

#endif