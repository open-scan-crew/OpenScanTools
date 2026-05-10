#ifndef CONTEXT_EXPORT_DXF_H
#define CONTEXT_EXPORT_DXF_H

#include "controller/functionSystem/AContext.h"
#include "io/exports/dxfExport.h"
#include "models/graph/AGraphNode.h"
#include "io/exports/ExportParameters.hpp"


struct Measure;

class ContextExportDxf : public AContext, public dxfExport
{
public:
	enum ErrorType { Success, WrongType, FailedToOpen, FailedToWrite };
public:
	ContextExportDxf(const ContextId& id);
	~ContextExportDxf();
	ContextState start(Controller& controller);
	ContextState launch(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller);
	bool canAutoRelaunch() const;
	ContextType getType() const override; 

private:
	ContextState processError(const ErrorType& error, Controller& controller);
	ErrorType getFilename(const ElementType& type, std::filesystem::path& output) const;
	ErrorType exportMarker(const std::filesystem::path& output, const glm::dvec3& center, const std::wstring& text);
	ErrorType exportLines(const std::filesystem::path& output, const std::vector<Measure>& lines, const glm::dvec3& decalExport, const std::wstring& text);
	ErrorType exportWireframeObject(const std::filesystem::path& output, const glm::dmat4& model,const std::vector<glm::dvec3>& vertices, const std::vector<uint32_t>& edgesIndices, const std::wstring& text);

private:
	std::unordered_set<SafePtr<AGraphNode>>			m_listToExport;
	std::filesystem::path				m_output;
	PrimitivesExportParameters			m_primitiveExportParam;
};

#endif // !CONTEXT_EXPORT_DXF_H_
