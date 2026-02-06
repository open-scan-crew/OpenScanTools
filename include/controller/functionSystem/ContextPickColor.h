#ifndef CONTEXT_PICK_COLOR_H
#define CONTEXT_PICK_COLOR_H

#include "controller/functionSystem/ARayTracingContext.h"

class ContextPickColor : public ARayTracingContext
{
public:
    explicit ContextPickColor(const ContextId& id);
    ~ContextPickColor() override;

    ContextState start(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    ContextState launch(Controller& controller) override;
    bool canAutoRelaunch() const override;
    ContextType getType() const override;
};

#endif // CONTEXT_PICK_COLOR_H
