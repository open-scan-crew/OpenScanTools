#ifndef CONTEXT_EXPORT_STEP_H_
#define CONTEXT_EXPORT_STEP_H_

#include "controller/functionSystem/AContext.h"
#include "models/3d/Graph/AGraphNode.h"
#include "io/exports/stepExport.h"
#include <filesystem>
#include <glm/glm.hpp>
#include "io/exports/ExportParameters.hpp"

struct Measure;

class ContextExportStep : public AContext, public stepExport
{
public:
	enum class ErrorType { Success, WrongType, FailedToOpen, FailedToWrite };
public:
	ContextExportStep(const ContextId& id);
	~ContextExportStep();
	ContextState start(Controller& controller);
	ContextState launch(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller);

	void addTypeDirectory(const ElementType& type);
	void getTypeNameDirectory(const ElementType& type, ElementType& directoryType, std::wstring& directoryName);
	bool addPipingDirectory(const xg::Guid& pipingId, Controller& controller);

	bool canAutoRelaunch() const;
	ContextType getType() const override; 

private:
	ContextState processError(const ErrorType& error, Controller& controller);
	//ErrorType getFilename(const ElementType& type, std::filesystem::path& output) const;

private:
	std::unordered_set<SafePtr<AGraphNode>>			m_listToExport;
	std::filesystem::path				m_output;
	std::unordered_map<ElementType, TDF_Label> m_typesDirectory;
	//std::unordered_map<const Piping*, TDF_Label> m_pipingDirectory;

	PrimitivesExportParameters m_parameters;
};

#endif // !CONTEXT_EXPORT_STEP_H_
