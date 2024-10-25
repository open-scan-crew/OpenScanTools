#ifndef CONTEXT_EXPORT_FBX_H
#define CONTEXT_EXPORT_FBX_H

#include "controller/functionSystem/AContext.h"
#include "utils/safe_ptr.h"
#include "io/exports/ExportParameters.hpp"

#include <filesystem>
#include <unordered_set>
#include <fbxsdk.h>

class MeshBuffer;
class Color32;
class AGraphNode;

struct RawMeshData;
struct Measure;

enum class ElementType;

class ContextExportFbx : public AContext
{
public:
	ContextExportFbx(const ContextId& id);
	~ContextExportFbx();
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

	FbxMesh* createMesh(FbxScene* output, const std::wstring& meshName, const RawMeshData& geom);

	void addObject(FbxScene* output, FbxMesh* mesh, const glm::mat4& transfo, const std::wstring& name, const Color32& color);
	void addObject(FbxScene* output, const std::vector<std::vector<glm::dvec3>>& mesures, const glm::mat4& transfo, const std::wstring& name, const Color32& color);

private:
	std::unordered_set<SafePtr<AGraphNode>>					m_listToExport;
	std::filesystem::path									m_output;
	PrimitivesExportParameters								m_primitiveExportParam;
};

#endif // !CONTEXT_EXPORT_DXF_H_
