#ifndef CONTEXT_EXPORT_OBJ_H
#define CONTEXT_EXPORT_OBJ_H

#include "controller/functionSystem/AContext.h"
#include "utils/safe_ptr.h"
#include "io/exports/ExportParameters.hpp"

#include <filesystem>
#include <unordered_set>

class MeshBuffer;
struct RawMeshData;
class AGraphNode;

class ContextExportObj : public AContext
{
public:
	ContextExportObj(const ContextId& id);
	~ContextExportObj();
	ContextState start(Controller& controller);
	ContextState launch(Controller& controller);
	ContextState feedMessage(IMessage* message, Controller& controller);
	bool canAutoRelaunch() const;
	ContextType getType() const override; 

private:
	ContextState processError(Controller& controller);
	void meshBufferToRawMeshData(const std::shared_ptr<MeshBuffer>& meshBuffer, RawMeshData& meshData);
	void getRawIndicesData(void* bufferData, const std::shared_ptr<MeshBuffer>& meshBuffer, std::vector<uint32_t>& indexes);
	void getTrianglesList(const RawMeshData& meshData, std::vector<uint32_t>& indexes);
	void addPoint(std::ofstream& stream, const glm::dvec3& point);

private:
	std::unordered_set<SafePtr<AGraphNode>>		m_listToExport;
	std::filesystem::path						m_output;
	PrimitivesExportParameters					m_primitiveExportParam;
};

#endif // !CONTEXT_EXPORT_DXF_H_
