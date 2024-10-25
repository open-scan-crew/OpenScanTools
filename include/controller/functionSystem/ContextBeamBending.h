#ifndef CONTEXT_BEAM_BENDING_H_
#define CONTEXT_BEAM_BENDING_H_

#include "controller/functionSystem/ARayTracingContext.h"
#include "controller/functionSystem/PlaneDetectionOptions.h"


class ContextBeamBending: public ARayTracingContext
{
public:
	ContextBeamBending(const ContextId& id);
	~ContextBeamBending();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;

	bool canAutoRelaunch() const;
	ContextType getType() const override;
	void resetClickUsages();

private:
	BeamBendingOptions m_options;
};

#endif // !CONTEXT_BEAM_BENDING_H_
