#ifndef CONTEXT_DATA_GENERATION_H
#define CONTEXT_DATA_GENERATION_H

#include "controller/functionSystem/AContext.h"

#include "models/graph/AGraphNode.h"

class ContextDataGeneration : public AContext
{
public:
    ContextDataGeneration(const ContextId& id);
    ~ContextDataGeneration();
    ContextState start(Controller& controller);
    ContextState feedMessage(IMessage* message, Controller& controller);
    ContextState launch(Controller& controller);

    bool canAutoRelaunch() const;
    ContextType getType() const override;

private:
    void generateTags(Controller& controller, std::unordered_set<SafePtr<AGraphNode>>& ret_nodes);
    void generateSimpleMeasures(Controller& controller, std::unordered_set<SafePtr<AGraphNode>>& ret_nodes);
    void generatePolyMeasures(Controller& controller, std::unordered_set<SafePtr<AGraphNode>>& ret_nodes);

};

#endif // !CONTEXT_CREATE_TAG_H_
