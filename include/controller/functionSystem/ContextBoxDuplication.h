#ifndef CONTEXT_BOX_DUPLICATION_H
#define CONTEXT_BOX_DUPLICATION_H

#include "controller/functionSystem/ARayTracingContext.h"
#include "controller/functionSystem/ADuplication.h"

class ContextBoxDuplication : public ARayTracingContext, public ADuplication
{
public:
    ContextBoxDuplication(const ContextId& id);
    ~ContextBoxDuplication();
    ContextState start(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    ContextState launch(Controller& controller) override;
    bool canAutoRelaunch() const;
    ContextType getType() const;

};

#endif
