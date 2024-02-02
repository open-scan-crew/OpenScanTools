#ifndef CONTEXT_COLUMN_TILT_H
#define CONTEXT_COLUMN_TILT_H

#include "controller/functionSystem/ARayTracingContext.h"
#include <glm/glm.hpp>

class ContextColumnTilt : public ARayTracingContext
{
public:
    ContextColumnTilt(const ContextId& id);
    ~ContextColumnTilt();
    ContextState start(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    ContextState launch(Controller& controller) override;
    bool canAutoRelaunch() const;
    ContextType getType() const override;
};

#endif
