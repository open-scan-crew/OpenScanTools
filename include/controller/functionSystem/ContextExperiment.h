#ifndef CONTEXT_EXPERIMENT_H_
#define CONTEXT_EXPERIMENT_H_

#include "controller/functionSystem/ARayTracingContext.h"
#include "models/pointCloud/TLS.h"

#include <list>

class ContextExperiment : public ARayTracingContext
{
public:
	ContextExperiment(const ContextId& id);
	~ContextExperiment();
	ContextState start(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	ContextState launch(Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

private:
	bool			m_ready;
	ContextId		m_TEMPORARYsubFunctionID;
	std::list<tls::ScanGuid> m_visibleScans;

};

#endif // !CONTEXT_EXPERIMENT_H_
