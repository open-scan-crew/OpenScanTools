#ifndef ACONTEXT_ALIGN_VIEW_H_
#define ACONTEXT_ALIGN_VIEW_H_

#include "controller/functionSystem/ARayTracingContext.h"

class CameraNode;

class AContextAlignView : public ARayTracingContext
{
public:
	AContextAlignView(const ContextId& id);
	~AContextAlignView();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	virtual ContextState launch(Controller& controller) = 0;
	bool canAutoRelaunch() const;

	virtual ContextType getType() const = 0;

protected:
	SafePtr<CameraNode> m_cameraNode;
};

#endif // !ACONTEXT_ALIGN_VIEW_H_
