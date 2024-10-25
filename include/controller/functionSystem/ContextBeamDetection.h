#ifndef CONTEXT_BEAM_DETECTION_H_
#define CONTEXT_BEAM_DETECTION_H_

#include "controller/functionSystem/ARayTracingContext.h"

class ContextBeamDetection : public ARayTracingContext
{
public:
	ContextBeamDetection(const ContextId& id);
	~ContextBeamDetection();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

private:

};

#endif // !CONTEXT_BEAM_DETECTION_H_