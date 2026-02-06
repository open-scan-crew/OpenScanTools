#ifndef CONTEXT_PICK_COLORIMETRIC_H
#define CONTEXT_PICK_COLORIMETRIC_H

#include "controller/functionSystem/ARayTracingContext.h"

class ContextPickColorimetric : public ARayTracingContext
{
public:
    explicit ContextPickColorimetric(const ContextId& id);
    ~ContextPickColorimetric() override;

    ContextState start(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    ContextState launch(Controller& controller) override;
    bool canAutoRelaunch() const override;
    ContextType getType() const override;
};

#endif
