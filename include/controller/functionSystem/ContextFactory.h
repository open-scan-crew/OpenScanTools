#ifndef CONTEXT_FACTORY_H
#define CONTEXT_FACTORY_H

#include "controller/functionSystem/AContext.h"

class ContextFactory
{
public:
	ContextFactory();
	~ContextFactory();

	AContext* createContext(const ContextType& type, ContextId& id, const ContextId& parent = INVALID_CONTEXT_ID);
};

#endif // !CONTEXT_FACTORY_H
