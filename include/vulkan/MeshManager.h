#ifndef MESH_MANAGER_H
#define MESH_MANAGER_H

#include "vulkan/Graph/MemoryReturnCode.h"
#include "models/3d/MeshBuffer.h"
#include "io/MeshObjectTypes.h"

#include <glm/glm.hpp>

#include <map>
#include <set>
#include <unordered_set>
#include <filesystem>

#include "utils/safe_ptr.h"

class Controller;
class IMeshReader;
class MeshObjectNode;
class ClusterNode;

struct ProjectionFrustum;
struct FBXWritingInfo;

struct RawMeshData
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<uint32_t> indices;
	std::unordered_map<VkPrimitiveTopology, std::vector<StandardDraw>> standardDraws;
	std::unordered_map<VkPrimitiveTopology, std::vector<IndexedDraw>> indexedDraws;

	void addDraws(VkPrimitiveTopology topology, const std::vector<StandardDraw>& draws)
	{
		if (standardDraws.find(topology) != standardDraws.end())
		{
			std::vector<StandardDraw>& drawList = standardDraws.at(topology);
			drawList.insert(drawList.end(), draws.begin(), draws.end());
		}
		else
		{
			standardDraws.insert({ topology, draws });
		}
	}

	void addDraw(VkPrimitiveTopology topology, const StandardDraw& draw)
	{
		addDraws(topology, { draw });
	}

	void addIndexedDraws(VkPrimitiveTopology topology, const std::vector<IndexedDraw>& draws)
	{

		if (indexedDraws.find(topology) != indexedDraws.end())
		{
			std::vector<IndexedDraw>& drawList = indexedDraws.at(topology);
			drawList.insert(drawList.end(), draws.begin(), draws.end());
		}
		else
		{
			indexedDraws.insert({ topology, draws });
		}
	}

	void addIndexedDraw(VkPrimitiveTopology topology, const IndexedDraw& draw)
	{
		addIndexedDraws(topology, { draw });
	}
};

struct SMesh
{
	std::shared_ptr<MeshBuffer>	m_mesh;
	std::wstring		m_name;
	MeshId				m_id;
	glm::vec3			m_dimensions;
	glm::vec3			m_center;
};


struct MeshObjOutputData 
{
	struct MeshObjectInfo
	{
		std::filesystem::path path;
		SafePtr<ClusterNode> parentCluster;
	};

	std::map<MeshId, MeshObjectInfo> meshIdInfo;
	glm::vec3 mergeCenter;
	glm::vec3 mergeDim;
	float scale;

};

struct HashMeshObjInput {
	size_t operator() (const MeshObjInputData& key) const {
		std::wstring hashString;
		hashString += key.path.wstring();
		hashString += key.extension;
		hashString += key.lod;

		hashString += key.generateEdges;
		hashString += key.isMerge;

		return std::hash<std::wstring>{}(hashString);
	}
};


enum class ManipulationMode;
enum class Selection;

class MeshManager
{
public:
	static MeshManager& getInstance()
	{
		static MeshManager instance;
		return instance;
	}
	void cleanMemory(bool onlyProjectMemory);
	void cleanSimpleGeometryMemory();

	static const std::map<GenericMeshType, std::map<ManipulationMode, bool>> SimpleObjectAcceptableManipulators;
	static const std::map<GenericMeshType, std::map<ManipulationMode, std::unordered_set<Selection>>> SimpleObjectManipulatorSelections;

	ObjectAllocation::ReturnCode loadFile(MeshObjOutputData& data, const MeshObjInputData& input, bool globalMesh = false, const std::filesystem::path& folderOutputPath = "", Controller* controller = nullptr);
	ObjectAllocation::ReturnCode reloadMeshFile(MeshObjectNode& meshNode, std::filesystem::path folder = "", Controller* controller = nullptr);

	MeshId getNewMeshId();

	static std::shared_ptr<MeshBuffer> getGenericMesh(GenericMeshId meshId);
	SMesh const getMesh(const MeshId& id);
	std::shared_ptr<MeshBuffer> getManipMesh(ManipulationMode manipMod);
	bool isMeshLoaded(const MeshId& id);
	bool addMeshInstance(const MeshId& id);
	uint64_t getMeshCounters(const MeshId& id);

	glm::vec3 getMeshDimension(const MeshId& id);

	ObjectAllocation::ReturnCode getBoxId(GenericMeshId& cubeId);
	ObjectAllocation::ReturnCode getCylinderId(GenericMeshId& cylinderId);
	ObjectAllocation::ReturnCode getSphereId(GenericMeshId& sphereId);
	ObjectAllocation::ReturnCode getTorusId(MeshId& torusId, const float& mainAngleRad, const float& mainRadius, const float& tubeRadius);

	ObjectAllocation::ReturnCode getBasicCameraModel(GenericMeshId& frustumMeshId);
	ObjectAllocation::ReturnCode getPerspectiveCameraModel(const ProjectionFrustum& box, GenericMeshId& frustumMeshId);
	ObjectAllocation::ReturnCode getOrthographicCameraModel(const ProjectionFrustum& box, GenericMeshId& frustumMeshId);

	bool removeGenericMeshInstance(GenericMeshId id);
	bool removeMeshInstance(MeshId id);
	void removeGridMesh(const std::shared_ptr<MeshBuffer>& meshBuff);

	RawMeshData generateBox();
	RawMeshData generateCylinder(const uint32_t& pointsInCircles);
	RawMeshData generateSphere(const uint32_t& sectorCount, const uint32_t& stackCount);
	RawMeshData generateTorusPart(uint32_t sectorCount, float angleRad, float ratioInnerOuterRadius);

	VkResult uploadInMeshBuffer(MeshBuffer& obj, const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals, const std::vector<uint32_t>& indices);

	void emptyTrash();

protected:
	MeshManager();
	~MeshManager();

	void initManipMesh();

	ObjectAllocation::ReturnCode loadExternModel(MeshObjOutputData& data, const MeshObjInputData& input, bool globalMesh, IMeshReader* reader, const std::filesystem::path& folderOutputPath, Controller* controller);

	std::filesystem::path writeInternFile(const FBXWritingInfo& _info, const MeshGeometries& geometrie, const std::filesystem::path& folderOutput, float scale);

	ObjectAllocation::ReturnCode checkGenericMeshAllocation(GenericMeshId id);
	ObjectAllocation::ReturnCode generateGenericMesh(GenericMeshId id);

	ObjectAllocation::ReturnCode allocateMeshBuffer(const std::shared_ptr<MeshBuffer>& meshBuffer, const RawMeshData& meshData);

	RawMeshData generateBasicCameraModel();
	RawMeshData generatePerspectiveCameraModel(const ProjectionFrustum& box);
	RawMeshData generateOrthographicCameraModel(const ProjectionFrustum& box);

	void removeGenericMesh(GenericMeshId id);
	void removeMesh(MeshId id);

protected:
	std::unordered_map<GenericMeshId, std::shared_ptr<MeshBuffer>>	m_genericMeshes;
	std::unordered_map<GenericMeshId, uint32_t>		m_simpleMeshCounters;
	std::unordered_map<ManipulationMode, MeshId>	m_manipsMesh;
	std::map<MeshId, SMesh>							m_meshes;
	std::set<MeshId>								m_globalMeshes;
	std::map<MeshId, uint32_t>						m_meshesCounters;
	std::map<std::size_t, MeshObjOutputData>		m_loaded;

	std::vector<std::shared_ptr<MeshBuffer>>		m_meshTrash;
	std::mutex										m_trashMutex;
};

#endif // !MESH_MANAGER_H_
