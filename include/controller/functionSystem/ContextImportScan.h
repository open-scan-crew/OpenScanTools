#ifndef CONTEXT_IMPORT_SCAN_H_
#define CONTEXT_IMPORT_SCAN_H_

#include "controller/functionSystem/AContext.h"

#include <filesystem>
#include <vector>
#include <qstring.h>

class ContextImportScan: public AContext
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
	std::vector<std::filesystem::path>		m_inputFiles;
	uint64_t								m_currentStep;
};

#endif // !CONTEXT_IMPORT_SCAN_H_