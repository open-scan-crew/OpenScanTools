#ifndef CONTEXT_VIEWPOINT_H_
#define CONTEXT_VIEWPOINT_H_

#include "controller/functionSystem/AContext.h"
#include "models/OpenScanToolsModelEssentials.h"

class ViewPointNode;
class CameraNode;

class ContextViewPointCreation : public AContext
{
public:
	ContextViewPointCreation(const ContextId& id);
	~ContextViewPointCreation();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	virtual ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;	

	virtual ContextType getType() const override;

private:
	SafePtr<CameraNode> m_cameraNode;
};

class ContextViewPointUpdate : public AContext
{
public:
	ContextViewPointUpdate(const ContextId& id);
	~ContextViewPointUpdate();
	ContextState start(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	virtual ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	virtual ContextType getType() const override;

private:
	uint8_t		m_messageCount;
	SafePtr<CameraNode>		m_cameraNode;
	SafePtr<ViewPointNode>		m_viewpoint;

};

#endif // !CONTEXT_VIEWPOINT_H_
