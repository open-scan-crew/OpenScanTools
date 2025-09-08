#ifndef CONTEXT_POINT_CLOUD_OBJECT_DUPLICATION_H
#define CONTEXT_POINT_CLOUD_OBJECT_DUPLICATION_H

#include "controller/functionSystem/ARayTracingContext.h"
#include "controller/functionSystem/ADuplication.h"

class PointCloudNode;

class ContextPCODuplication : public ARayTracingContext, public ADuplication
{
public:
	ContextPCODuplication(const ContextId& id);
	~ContextPCODuplication();
	ContextState start(Controller& controller);
	ContextState launch(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller) override;
    bool canAutoRelaunch() const;
	ContextType getType() const override;

private:
	SafePtr<PointCloudNode> m_pco;

};

#endif
