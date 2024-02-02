#ifndef CONTEXT_FIND_SCAN_H_
#define CONTEXT_FIND_SCAN_H_

#include "controller/functionSystem/ARayTracingContext.h"

class ContextFindScan : public ARayTracingContext
{
public:
	ContextFindScan(const ContextId& id);
	~ContextFindScan();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller);
	ContextState launch(Controller& controller);

	bool canAutoRelaunch() const;
	ContextType getType() const override;

};

#endif // !CONTEXT_CREATE_TAG_H_
