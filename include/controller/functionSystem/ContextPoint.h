#ifndef CONTEXT_POINT_H_
#define CONTEXT_POINT_H_

#include "controller/functionSystem/ARayTracingContext.h"

class ContextPoint : public ARayTracingContext
{
public:
	ContextPoint(const ContextId& id);
	~ContextPoint();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	virtual ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	virtual ContextType getType() const override;

};

#endif // !CONTEXT_POINT_H_
