#ifndef CONTEXT_ALIGN_VIEWBOX_H
#define CONTEXT_ALIGN_VIEWBOX_H

#include "controller/functionSystem/AContext.h"

#include "utils/safe_ptr.h"

class CameraNode;
class TransformationModule;


class ContextAlignViewBox : public AContext
{
public:
	ContextAlignViewBox(const ContextId& id);
	~ContextAlignViewBox();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;

	bool canAutoRelaunch() const;

	ContextType getType() const override;
protected:
	void alignView(Controller& controller, const TransformationModule& module);

protected:
	SafePtr<CameraNode>	m_cameraNode;
};

#endif
