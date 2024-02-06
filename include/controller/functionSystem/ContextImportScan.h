#ifndef CONTEXT_IMPORT_SCAN_H_
#define CONTEXT_IMPORT_SCAN_H_

#include "controller/functionSystem/ARayTracingContext.h"

#include <qstring.h>
#include "io/imports/ImportTypes.h"



class ContextImportScan: public ARayTracingContext
{
public:
	ContextImportScan(const ContextId& id);
	~ContextImportScan();
	ContextState start(Controller& controller) override;
	ContextState launch(Controller& controller) override;
	ContextState feedMessage(IMessage* message, Controller& controller) override;
	bool canAutoRelaunch() const;

	ContextType getType() const override;

private:
	void updateStep(Controller& controller, const QString& state, const uint64_t& step);

private:
	Import::ScanInfo						m_scanInfo;
	uint64_t								m_currentStep;
};

#endif // !CONTEXT_IMPORT_SCAN_H_