#ifndef CONTEXT_POINT_CREATION_H
#define CONTEXT_POINT_CREATION_H

#include "controller/functionSystem/ARayTracingContext.h"

class ContextPointCreation : public ARayTracingContext
{
public:
	ContextPointCreation(const ContextId& id);
	~ContextPointCreation();
	ContextState launch(Controller& controller) override final;
	ContextType getType() const override;
    bool canAutoRelaunch() const;
};

#endif