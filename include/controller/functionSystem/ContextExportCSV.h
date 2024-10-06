#ifndef CONTEXT_EXPORT_CSV_H_
#define CONTEXT_EXPORT_CSV_H_

#include "controller/functionSystem/AContext.h"
#include <filesystem>
#include "models/graph/AGraphNode.h"
#include "io/exports/ExportParameters.hpp"

struct Measure;

class ContextExportCSV : public AContext
{
public:
	enum ErrorType { Success, WrongType, FailedToOpen, FailedToWrite };
public:
	ContextExportCSV(const ContextId& id);
	~ContextExportCSV();
	ContextState start(Controller& controller);
	ContextState launch(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller);
	bool canAutoRelaunch() const;
	ContextType getType() const override;

private:
	ContextState processError(const ErrorType& error, Controller& controller);
	ErrorType getFilename(const ElementType& type, std::filesystem::path& output) const;
	ErrorType addTypeExtFilename(const ElementType& type, std::wstring& fileName) const;

private:
	std::unordered_set<SafePtr<AGraphNode>>		m_listToExport;
	std::filesystem::path						m_output;
	PrimitivesExportParameters					m_primitiveExportParam;
};

#endif // !CONTEXT_EXPORT_CSV_H_
