#ifndef _I_MESH_FILE_READER_H_
#define _I_MESH_FILE_READER_H_

#include "io/MeshObjectTypes.h"
#include "vulkan/Graph/MemoryReturnCode.h"
#include "utils/Logger.h"
#include "models/3d/Graph/ClusterNode.h"
#include "models/OpenScanToolsModelEssentials.h"

#define IOLOG Logger::log(LoggerMode::IOLog)

namespace ObjectAllocation {
	enum class ReturnCode;
}

class Controller;
class MeshBuffer;

struct MeshShape
{
	std::wstring name;
	MeshGeometries geometry;

	glm::vec3 dim;
	glm::vec3 center;

	SafePtr<ClusterNode> parentCluster;
};

class IMeshReader
{
public:
	IMeshReader(Controller* pController, const MeshObjInputData& inputData);

	virtual bool read() = 0;
	virtual ObjectAllocation::ReturnCode generateGeometries() = 0;
	virtual xg::Guid getLoadedMeshId() const;

	int& editLoadCountUI();
	int& editMaxCountUI();

	// return false if the context is aborted by the user
	void createImportProcessUI(QString loadText, bool enableCancel);
	bool updateImportProcessUI(QString loadText, bool enableCancel);
	ObjectAllocation::ReturnCode allocateMeshBuffer(MeshBuffer& _mesh, const MeshShape& shape);

	std::vector<MeshShape>& getMeshShapes();

	void getCorrectedVertices(std::vector<float>& buffer, const glm::vec3& translation) const;
	void fillMeshesBoundingBox(glm::vec3& merge_dim, glm::vec3& merge_center);

	float getScale() const;

	static void getBoundingBox(const std::vector<float>& vertices, glm::vec3& dim, glm::vec3& center, std::array<glm::vec3, 2>& merge_bound);
	static void checkBoundingBox(const glm::vec3& vertex, std::array<glm::vec3, 2>& dim);

protected:

	virtual ObjectAllocation::ReturnCode virtualAllocateMeshBuffer(MeshBuffer& _mesh, const MeshShape& shape);
	static ObjectAllocation::ReturnCode allocateMesh(MeshBuffer* _mesh, const MeshGeometries& geometrie);
	
protected:
	std::vector<MeshShape> m_meshesShapes;

	Controller* m_pController;
	MeshObjInputData m_inputInfo;

	int	m_loadCount;
	int m_maxCount;

	float m_scale;

};

#endif // !_STEP_FILE_READER_H_
