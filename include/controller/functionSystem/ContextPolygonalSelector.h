#ifndef CONTEXT_POLYGONAL_SELECTOR_H
#define CONTEXT_POLYGONAL_SELECTOR_H

#include "controller/functionSystem/AContext.h"
#include "models/3d/DisplayParameters.h"
#include <vector>

class ContextPolygonalSelector : public AContext
{
public:
    ContextPolygonalSelector(const ContextId& id);
    ~ContextPolygonalSelector();

    ContextState start(Controller& controller) override;
    ContextState feedMessage(IMessage* message, Controller& controller) override;
    ContextState launch(Controller& controller) override;
    ContextState abort(Controller& controller) override;
    ContextState validate(Controller& controller) override;
    bool canAutoRelaunch() const override;
    ContextType getType() const override;

private:
    std::vector<glm::vec2> m_currentVertices;
    PolygonalSelectorSettings m_settings;
};

#endif
