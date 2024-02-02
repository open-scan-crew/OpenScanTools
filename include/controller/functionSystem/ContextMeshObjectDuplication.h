#ifndef CONTEXT_WAVEFRONT_DUPLICATION_H
#define CONTEXT_WAVEFRONT_DUPLICATION_H

#include "controller/functionSystem/ARayTracingContext.h"
#include "controller/functionSystem/ADuplication.h"

class MeshObjectNode;

class ContextMeshObjectDuplication : public ARayTracingContext, public ADuplication
{
public:
	ContextMeshObjectDuplication(const ContextId& id);
	~ContextMeshObjectDuplication();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller) override;
    ContextState launch(Controller& controller) override;
    bool canAutoRelaunch() const;
	ContextType getType() const;
};

#endif
