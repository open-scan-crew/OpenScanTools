#include "vulkan/MeshManager.h"
#include "vulkan/VulkanManager.h"
#include "vulkan/Graph/MemoryReturnCode.h"
#include <glm/gtc/constants.hpp>

#ifndef PORTABLE
#include "io/imports/objFileReader.h"
#include "io/imports/stepFileReader.h"
#include "io/imports/ifcFileReader.h"
#include "io/imports/dxfFileReader.h"
#endif

// Config.h must be include before any fbx include due to a redefinition of snprintf :(
#include "utils/Config.h"
#include "io/imports/fbxWriterReader.h"

#include "models/3d/ProjectionData.h"

#include "models/3d/ManipulationTypes.h"

#include "gui/GuiData/GuiDataMessages.h"
#include "gui/texts/SplashScreenTexts.hpp"

#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "controller/functionSystem/FunctionManager.h"

#include "models/graph/MeshObjectNode.h"

#include "utils/System.h"

#include "io/FileUtils.h"
#include "utils/Logger.h"

const std::map<GenericMeshType, std::map<ManipulationMode, bool>>MeshManager::SimpleObjectAcceptableManipulators = {
	{ GenericMeshType::Cube, {
		{ManipulationMode::Translation, true},
		{ManipulationMode::Rotation, true},
		{ManipulationMode::Extrusion, true},
		{ManipulationMode::Scale, false}
	}},
	{GenericMeshType::Cylinder, {
		{ManipulationMode::Translation, true},
		{ManipulationMode::Rotation, false},
		{ManipulationMode::Extrusion, true},
		{ManipulationMode::Scale, true}
	}},
	{GenericMeshType::Sphere, {
		{ManipulationMode::Translation, true},
		{ManipulationMode::Rotation, false},
		{ManipulationMode::Extrusion, false},
		{ManipulationMode::Scale, false}
	}}
};

const std::map<GenericMeshType, std::map<ManipulationMode, std::unordered_set<Selection>>> MeshManager::SimpleObjectManipulatorSelections = {
	{GenericMeshType::Cube, {
		{ManipulationMode::Translation, { Selection::X, Selection::Y, Selection::Z }},
		{ManipulationMode::Rotation, { Selection::X, Selection::Y, Selection::Z }},
		{ManipulationMode::Extrusion, { Selection::X, Selection::Y, Selection::Z, Selection::_X, Selection::_Y, Selection::_Z }},
		{ManipulationMode::Scale, {}}
	}},
	{GenericMeshType::Cylinder, {
		{ManipulationMode::Translation, { Selection::X, Selection::Y, Selection::Z }},
		{ManipulationMode::Rotation, {}},
		{ManipulationMode::Extrusion, { Selection::Z, Selection::_Z }},
		{ManipulationMode::Scale, { Selection::XY }}
	}},
	{GenericMeshType::Sphere, {
		{ManipulationMode::Translation, { Selection::X, Selection::Y, Selection::Z }},
		{ManipulationMode::Rotation, {}},
		{ManipulationMode::Extrusion, {}},
		{ManipulationMode::Scale, {}}
	}}
};

MeshManager::MeshManager()
{
	initManipMesh();
}

MeshManager::~MeshManager()
{
	GRAPH_LOG << "Destroying all meshes" << Logger::endl;
	for (auto& pair : m_meshes)
	{
		//Quentin : cause l'arrêt car les counters des meshes des manipulateurs ne sont pas à zéro
		assert(m_meshesCounters[pair.first] == 0);
		VulkanManager::getInstance().freeAllocation(pair.second.m_mesh->m_smpBuffer);
	}
	m_meshes.clear();
	m_meshesCounters.clear();

	for (auto& pair : m_genericMeshes)
	{
		assert(m_simpleMeshCounters[pair.first] == 0);
		VulkanManager::getInstance().freeAllocation(pair.second->m_smpBuffer);
	}
	m_genericMeshes.clear();
	m_simpleMeshCounters.clear();
	m_loaded.clear();

	emptyTrash();
}

void MeshManager::initManipMesh()
{
	std::unordered_map<ManipulationMode, std::filesystem::path> manipulatorsObjects;

	std::filesystem::path dir(Config::getResourcesPath());
	manipulatorsObjects = std::unordered_map<ManipulationMode, std::filesystem::path>({
			{ManipulationMode::Translation, dir / "meshes/axis_arrow.obj"},
			{ManipulationMode::Rotation, dir / "meshes/rotation_ring.obj"},
			{ManipulationMode::Scale, dir / "meshes/extrusion_arrow.obj"},
			{ManipulationMode::Extrusion, dir / "meshes/extrusion_arrow.obj"}
		});

	std::unordered_map<ManipulationMode, MeshId> manips;
	for (const auto& obj : manipulatorsObjects)
	{
		MeshObjOutputData data;
		MeshObjInputData input(obj.second, false, true);
		input.centerPosition = false;

		ObjectAllocation::ReturnCode ret(loadFile(data, input, true, "", nullptr));


		if (ret != ObjectAllocation::ReturnCode::Success || data.meshIdInfo.size() != 1)
		{
			assert(false);
			return;
		}

		MeshId meshId = data.meshIdInfo.begin()->first;
		//assert(meshes.empty()==false);
		if (!meshId.isValid())
		{
			assert(false);
			return;
		}
		manips.insert({ obj.first, meshId });
	}

	m_manipsMesh = manips;
}

void MeshManager::cleanMemory(bool onlyProjectMemory)
{
	std::set<MeshId> toDeleteMeshes;
	for (auto& pair : m_meshes)
	{
		if (onlyProjectMemory && m_globalMeshes.find(pair.first) != m_globalMeshes.end())
			continue;
		//assert(m_meshesCounters[pair.first] == 0);
		if (m_meshesCounters[pair.first] > 0)
			continue;
		VulkanManager::getInstance().freeAllocation(pair.second.m_mesh->m_smpBuffer);
		toDeleteMeshes.insert(pair.first);
	}

	for (MeshId id : toDeleteMeshes)
	{
		m_meshes.erase(id);
		m_meshesCounters.erase(id);
	}

	std::vector<GenericMeshId> toDeleteSimplesObjects;
	for (auto& pair : m_genericMeshes)
	{
		//assert(m_simpleMeshCounters[pair.first] == 0);
		if (m_simpleMeshCounters[pair.first] > 0)
			continue;
		VulkanManager::getInstance().freeAllocation(pair.second->m_smpBuffer);
		toDeleteSimplesObjects.push_back(pair.first);
	}
	for (const GenericMeshId id : toDeleteSimplesObjects)
	{
		m_genericMeshes.erase(id);
		m_simpleMeshCounters.erase(id);
	}

	m_loaded.clear();
}

void MeshManager::cleanSimpleGeometryMemory()
{
	std::vector<GenericMeshId> toDeleteSimplesObjects;
	for (auto& pair : m_genericMeshes)
	{
		assert(m_simpleMeshCounters[pair.first] == 0);
		if (m_simpleMeshCounters[pair.first] > 0)
			continue;
		VulkanManager::getInstance().freeAllocation(pair.second->m_smpBuffer);
		toDeleteSimplesObjects.push_back(pair.first);
	}

	for (const GenericMeshId id : toDeleteSimplesObjects)
	{
		m_genericMeshes.erase(id);
		m_simpleMeshCounters.erase(id);
	}
}

ObjectAllocation::ReturnCode MeshManager::loadFile(MeshObjOutputData& data, const MeshObjInputData& input, bool globalMesh, const std::filesystem::path& folderOutputPath, Controller* controller)
{
	HashMeshObjInput input_hash; 
	size_t objId = input_hash(input);
	if (m_loaded.find(objId) != m_loaded.end())
	{
		data = m_loaded[objId];
		return ObjectAllocation::ReturnCode::Success;
	}

	ObjectAllocation::ReturnCode ret;

	IMeshReader* reader;

	switch (input.extension) {
#ifndef PORTABLE
		case FileType::OBJ:
		{
			reader = new ObjFileReader(controller, input);
		}
		break;
		case FileType::STEP:
		{
			reader = new stepFileReader(controller, input);
		}
		break;
		case FileType::IFC:
		{
			reader = new ifcFileReader(controller, input);
		}
		break;
		case FileType::DXF:
		{
			reader = new dxfFileReader(controller, input);
		}
		break;
#endif
		case FileType::FBX:
		{
			reader = new fbxWriterReader(controller, input);
		}
		break;
		default:
		{
			return (ObjectAllocation::ReturnCode::Load_File_Error);
		}
		break;
	}

	ret = loadExternModel(data, input, globalMesh, reader, folderOutputPath, controller);
	delete reader;

	if (controller != nullptr)
	{
		controller->updateInfo(new GuiDataSplashScreenEnd(GuiDataSplashScreenEnd::SplashScreenType::Message));
		controller->updateInfo(new GuiDataProcessingSplashScreenForceClose());
	}

	if (ret == ObjectAllocation::ReturnCode::Success)
	{
		m_loaded[objId] = data;
		for (const std::pair<MeshId, MeshObjOutputData::MeshObjectInfo>& meshOutput : data.meshIdInfo)
		{
			MeshObjOutputData newData;
			newData.mergeCenter = data.mergeCenter;
			newData.meshIdInfo[meshOutput.first] = meshOutput.second;

			std::filesystem::path meshPath = meshOutput.second.path;

			MeshObjInputData newInput(meshPath, true, true);
			size_t otherObjId = input_hash(newInput);
			m_loaded[otherObjId] = newData;
		}
	}

	return ret;
}

ObjectAllocation::ReturnCode MeshManager::reloadMeshFile(MeshObjectNode& meshNode, std::filesystem::path folder, Controller* controller)
{
	std::filesystem::path meshFilePath;
	if (!folder.empty())
		meshFilePath = folder / meshNode.getFilePath().filename();
	else
		meshFilePath = meshNode.getFilePath();

	assert(FileUtils::getType(meshFilePath) == FileType::FBX);
	if (!std::filesystem::exists(meshFilePath) 
		|| FileUtils::getType(meshFilePath) != FileType::FBX)
	{
		return ObjectAllocation::ReturnCode::Load_File_Error;
	}
	MeshObjInputData input(meshFilePath, true, true);
	input.centerPosition = false;

	if (controller != nullptr)
		controller->updateInfo(new GuiDataSplashScreenStart(TEXT_MESSAGE_SPLASH_SCREEN_READING_DATA_WITH_NAME.arg(QString::fromStdWString(meshFilePath.filename().wstring())), GuiDataSplashScreenStart::SplashScreenType::Message));
	
	MeshObjOutputData output;
	ObjectAllocation::ReturnCode ret = loadFile(output, input, false, "", nullptr);
	if (ret == ObjectAllocation::ReturnCode::Success)
	{
		MeshId id;
		if (output.meshIdInfo.size() == 1)
		{
			id = output.meshIdInfo.begin()->first;
			meshNode.setMeshId(id);
			addMeshInstance(id);
		}
		else
			assert(false);
		meshNode.setDimension(getMeshDimension(id));
	}
	else
		meshNode.setMeshId(xg::Guid());

	if (controller != nullptr)
	{
		controller->updateInfo(new GuiDataSplashScreenEnd(GuiDataSplashScreenEnd::SplashScreenType::Message));
		controller->updateInfo(new GuiDataProcessingSplashScreenForceClose());
	}

	return ret;
}

ObjectAllocation::ReturnCode MeshManager::loadExternModel(MeshObjOutputData& data, const MeshObjInputData& input, bool globalMesh, IMeshReader* reader, const std::filesystem::path& folderOutputPath, Controller* controller)
{
	if (controller != nullptr)
		controller->updateInfo(new GuiDataSplashScreenStart(TEXT_MESSAGE_SPLASH_SCREEN_READING_DATA_WITH_NAME.arg(QString::fromStdWString(input.path.filename().wstring())), GuiDataSplashScreenStart::SplashScreenType::Message));

	if (!reader->read())
	{
		IOLOG << "Error : failed to load " << input.path << Logger::endl;
		if (controller != nullptr)
			controller->updateInfo(new GuiDataSplashScreenEnd(GuiDataSplashScreenEnd::SplashScreenType::Message));
		return ObjectAllocation::ReturnCode::Load_File_Error;
	}
	if (controller != nullptr)
		controller->updateInfo(new GuiDataSplashScreenEnd(GuiDataSplashScreenEnd::SplashScreenType::Message));

	MeshId internFileMeshId = reader->getLoadedMeshId();

	bool isWritingFile = !folderOutputPath.empty() && (input.path.parent_path() != folderOutputPath || !internFileMeshId.isValid());

	int baseLoadCount = reader->editMaxCountUI();
	reader->editMaxCountUI() = baseLoadCount * (2 + isWritingFile * 1);
	reader->createImportProcessUI(TEXT_SPLASH_SCREEN_IMPORT_GEOMETRIES.arg(0).arg(baseLoadCount), true);

	ObjectAllocation::ReturnCode ret(reader->generateGeometries());
	if (ret != ObjectAllocation::ReturnCode::Success)
		return ret;

	if (reader->getMeshShapes().empty())
		return ObjectAllocation::ReturnCode::Load_File_Error;

	reader->editLoadCountUI() = baseLoadCount;
	reader->updateImportProcessUI(TEXT_SPLASH_SCREEN_IMPORT_PROCESSING, false);

	glm::vec3 merge_dim, merge_center;
	reader->fillMeshesBoundingBox(merge_dim, merge_center);

	std::vector<std::pair<FBXWritingInfo, const MeshGeometries&>> writingsInfo;

	for (MeshShape& shape : reader->getMeshShapes())
	{
		if (shape.geometry.vertices.size() != shape.geometry.normals.size())
			shape.geometry.normals.clear();

		if (shape.geometry.vertices.empty())
			continue;

		MeshId mId = internFileMeshId;
		if (!internFileMeshId.isValid())
			mId = getNewMeshId();

		if (isMeshLoaded(internFileMeshId))
		{
			data.meshIdInfo[mId] = MeshObjOutputData::MeshObjectInfo();

			FBXWritingInfo info;
			info.meshId = mId;
			info.name = shape.name;
			writingsInfo.push_back({ info, shape.geometry });
			continue;
		}

		SMesh newMesh;

		newMesh.m_name = shape.name;
		newMesh.m_dimensions = shape.dim;
		newMesh.m_center = shape.center - merge_center;
		newMesh.m_id = mId;
		newMesh.m_mesh = std::make_shared<MeshBuffer>();

		if(input.centerPosition)
			reader->getCorrectedVertices(shape.geometry.vertices, shape.center);

		ObjectAllocation::ReturnCode ret(reader->allocateMeshBuffer(*newMesh.m_mesh, shape));
		if (ret == ObjectAllocation::ReturnCode::MeshAllocFail)
			continue;
		else if (ret != ObjectAllocation::ReturnCode::Success)
			return ret;

		m_meshes[mId] = newMesh;
		if (globalMesh)
			m_globalMeshes.insert(mId);

		MeshObjOutputData::MeshObjectInfo meshObjInfo;
		meshObjInfo.parentCluster = shape.parentCluster;
		data.meshIdInfo[mId] = meshObjInfo;

		FBXWritingInfo info;
		info.meshId = mId;
		info.name = shape.name;
		writingsInfo.push_back({ info, shape.geometry });

		GRAPH_LOG << "Mesh loaded " << shape.name << Logger::endl;
	}

	if (data.meshIdInfo.empty())
		return ObjectAllocation::ReturnCode::Failed;

	float scale = input.meshScale > 0 ? input.meshScale : reader->getScale();
	scale = scale > 0 ? scale : 1;

	//Writing
	if (isWritingFile)
	{
		reader->editLoadCountUI() = baseLoadCount * 2;
		reader->updateImportProcessUI(TEXT_SPLASH_SCREEN_WRITE_INTERN_FILE_PROCESSING, false);

		for (auto writingInfo : writingsInfo)
		{
			std::filesystem::path filepath = writeInternFile(writingInfo.first, writingInfo.second, folderOutputPath, scale);
			if (controller != nullptr)
			{
				if (filepath.empty())
					controller->updateInfo(new GuiDataProcessingSplashScreenLogUpdate(TEXT_SPLASH_SCREEN_WRITE_INTERN_FILE_ERROR.arg(QString::fromStdWString(writingInfo.first.name))));
			}

			MeshId meshId = writingInfo.first.meshId;
			if (data.meshIdInfo.find(meshId) == data.meshIdInfo.end())
				data.meshIdInfo[meshId] = MeshObjOutputData::MeshObjectInfo();
			data.meshIdInfo[meshId].path = filepath;
		}

		if(controller != nullptr)
			controller->updateInfo(new GuiDataProcessingSplashScreenLogUpdate(TEXT_SPLASH_SCREEN_WRITE_INTERN_FILE_RESULT.arg(writingsInfo.size()).arg(QString::fromStdWString(folderOutputPath.wstring()))));

		reader->editLoadCountUI() = baseLoadCount * 3;
		reader->updateImportProcessUI(TEXT_SPLASH_SCREEN_WRITE_INTERN_FILE_PROCESSING, false);
	}
	else
	{
		assert(writingsInfo.size() == 1);
		assert(!input.path.filename().empty());

		MeshId meshId = writingsInfo.back().first.meshId;
		if (data.meshIdInfo.find(meshId) == data.meshIdInfo.end())
			data.meshIdInfo[meshId] = MeshObjOutputData::MeshObjectInfo();
		data.meshIdInfo[meshId].path = input.path.filename();
	}

	data.mergeDim = merge_dim;
	data.mergeCenter = merge_center;
	data.scale = scale;

	return ObjectAllocation::ReturnCode::Success;
}

std::filesystem::path MeshManager::writeInternFile(const FBXWritingInfo& _info, const MeshGeometries& geometrie, const std::filesystem::path& folderOutput, float scale)
{
	FBXWritingInfo info = _info;
	std::wstring filename;
	std::wstring idString = Utils::from_utf8(info.meshId.str());
	filename = info.name + L"_" + idString + L".fbx";

	Utils::System::formatFilename(filename);

	info.outputPath = folderOutput / filename;

	if (!fbxWriterReader::writeInternFbxFile(info, geometrie, scale))
		return L"";

	return info.outputPath;
}

MeshId MeshManager::getNewMeshId()
{
	return xg::newGuid();
}

std::shared_ptr<MeshBuffer> MeshManager::getGenericMesh(GenericMeshId meshId)
{
	MeshManager& manager = MeshManager::getInstance();
	if (manager.m_genericMeshes.find(meshId) != manager.m_genericMeshes.end())
		return manager.m_genericMeshes.at(meshId);
	else
		return std::shared_ptr<MeshBuffer>();
}

SMesh const MeshManager::getMesh(const MeshId& id)
{
	if (m_meshes.find(id) == m_meshes.end())
		return SMesh();
	return m_meshes.at(id);
}

std::shared_ptr<MeshBuffer> MeshManager::getManipMesh(ManipulationMode manipMode)
{
	if (m_manipsMesh.find(manipMode) != m_manipsMesh.end())
		return getMesh(m_manipsMesh.at(manipMode)).m_mesh;
	return std::shared_ptr<MeshBuffer>();
}

glm::vec3 MeshManager::getMeshDimension(const MeshId& id)
{
	SMesh smesh = getMesh(id);
	if (!smesh.m_mesh)
		return glm::vec3(NAN);
	return smesh.m_dimensions;
}

bool MeshManager::isMeshLoaded(const MeshId& id)
{
	return (m_meshes.find(id) != m_meshes.end());
}

bool MeshManager::addMeshInstance(const MeshId& id)
{
	if (m_meshes.find(id) == m_meshes.end())
		return false;
	else
		m_meshesCounters[id]++;
	return true;
}

uint64_t MeshManager::getMeshCounters(const MeshId& id)
{
	if (m_meshesCounters.find(id) == m_meshesCounters.end())
		return 0;
	return m_meshesCounters[id];
}

ObjectAllocation::ReturnCode MeshManager::getBoxId(GenericMeshId& id)
{
	id.type = GenericMeshType::Cube;
	id.shape.cube.faceCount = 6;
	return checkGenericMeshAllocation(id);
}

ObjectAllocation::ReturnCode MeshManager::getCylinderId(GenericMeshId& id) // Le rayon n'est pas utilisé, pourquoi le mettre?
{
	int pointsInCircles = 32;
	id.type = GenericMeshType::Cylinder;
	id.shape.cylinder.sectorCount = pointsInCircles;
	return checkGenericMeshAllocation(id);
}

ObjectAllocation::ReturnCode MeshManager::getSphereId(GenericMeshId& id)
{
	id.type = GenericMeshType::Sphere;
	int sectorCount = 32;
	int pointsInCircles = 32;
	id.shape.sphere.latitudeCount = sectorCount;
	id.shape.sphere.longitudeCount = pointsInCircles;
	return checkGenericMeshAllocation(id);
}

ObjectAllocation::ReturnCode MeshManager::getTorusId(MeshId& id, const float& mainAngleRad, const float& mainRadius, const float& tubeRadius)
{
	/*
	id.type = GenericMeshType::Torus;
	int sectorCount = 32;
	float outerRadius = mainRadius + tubeRadius;
	float innerRadius = mainRadius - tubeRadius;
	// On calcule la fraction la plus proche de innerRadius/outerRadius dont le dénominateur est < 255
	float ratio = innerRadius / outerRadius;
	uint8_t bestDenominator = 1;
	uint8_t bestNumerator = 1;
	float bestApprox = 1.f;
	for (uint8_t d = 1; d < 255; ++d)
	{
		uint8_t num = uint8_t(d * ratio);
		float approx = (float)num / (float)d;
		if (abs(ratio - approx) < abs(ratio - bestApprox))
		{
			bestDenominator = d;
			bestNumerator = num;
			bestApprox = approx;
		}
	}

	id.shape.torus.sectorCount = sectorCount;
	id.shape.torus.oneAndHalfDegreesCount = (int)std::round(mainAngleRad / M_PI * 120);
	id.shape.torus.innerRadius = bestNumerator;
	id.shape.torus.outerRadius = bestDenominator;
	*/

	float outerRadius = mainRadius + tubeRadius;
	float innerRadius = mainRadius - tubeRadius;
	MeshId meshId = getNewMeshId();
	SMesh smesh = SMesh();
	smesh.m_id = meshId;
	smesh.m_name = L"Torus";
	smesh.m_dimensions = glm::vec3(1.f, 1.f, 1.f);
	smesh.m_center = glm::vec3(0.f);
	smesh.m_mesh = std::make_shared<MeshBuffer>();

	ObjectAllocation::ReturnCode res = allocateMeshBuffer(smesh.m_mesh, generateTorusPart(32, mainAngleRad, innerRadius / outerRadius));

	if (res == ObjectAllocation::ReturnCode::Success)
	{
		id = meshId;
		m_meshes.insert({ meshId, smesh });
		m_meshesCounters.insert({ meshId, 1u });
	}

	return res;
}

ObjectAllocation::ReturnCode MeshManager::getBasicCameraModel(GenericMeshId& id)
{
	id.type = GenericMeshType::Camera_Perspective;
	/*
	id.parameters = glm::lowp_i8vec4(0);
	*/
	return checkGenericMeshAllocation(id);
}

ObjectAllocation::ReturnCode MeshManager::getPerspectiveCameraModel(const ProjectionFrustum& box, GenericMeshId& id)
{
	id.type = GenericMeshType::Camera_Perspective;
	//id.parameters = { box.r + box.l, box.t + box.b, box.f + box.n, 0.0f };
	return checkGenericMeshAllocation(id);
}

ObjectAllocation::ReturnCode MeshManager::getOrthographicCameraModel(const ProjectionFrustum& box, GenericMeshId& id)
{
	id.type = GenericMeshType::Camera_Orthographic;
	//id.parameters = { box.r + box.l, box.t + box.b, box.f + box.n, 1.0f };
	return checkGenericMeshAllocation(id);
}

/*void MeshManager::generateEdgesIndices(const std::vector<tinyobj::index_t>& indices, std::vector<uint32_t>& edgeIndices) const
{
	// Wireframe like indices list
	edgeIndices.reserve(indices.size());
	uint64_t size(indices.size() - (indices.size() % 3));
	for (uint64_t iterator(0); iterator < size;)
	{
		uint32_t id0(indices[iterator++].vertex_index), id1(indices[iterator++].vertex_index), id2(indices[iterator++].vertex_index);
		edgeIndices.push_back(id0);
		edgeIndices.push_back(id1);

		edgeIndices.push_back(id1);
		edgeIndices.push_back(id2);

		edgeIndices.push_back(id2);
		edgeIndices.push_back(id0);
	}
}	

*/

ObjectAllocation::ReturnCode MeshManager::checkGenericMeshAllocation(GenericMeshId id)
{
	if (m_genericMeshes.find(id) != m_genericMeshes.end())
	{
		m_simpleMeshCounters[id]++;
		return ObjectAllocation::ReturnCode::Success;
	}
	else
	{
		ObjectAllocation::ReturnCode code = generateGenericMesh(id);
		if (code == ObjectAllocation::ReturnCode::Success)
			m_simpleMeshCounters[id]++;
		return code;
	}
}

ObjectAllocation::ReturnCode MeshManager::generateGenericMesh(GenericMeshId id)
{
	ObjectAllocation::ReturnCode code = ObjectAllocation::ReturnCode::Failed;

	RawMeshData meshData;
	switch (id.type)
	{
	case GenericMeshType::Cube:
		// Version sans normales mais avec moins de vertices
		meshData = generateBox();
		// Version avec normales
		//meshData = generateIndexedBox();
		break;
	case GenericMeshType::Cylinder:
		meshData = generateCylinder(id.shape.cylinder.sectorCount);
		break;
	case GenericMeshType::Sphere:
		meshData = generateSphere(id.shape.sphere.latitudeCount, id.shape.sphere.longitudeCount);
		break;
	case GenericMeshType::Camera_Perspective:
		//meshData = generatePerspectiveCameraModel(box);
		break;
	case GenericMeshType::Camera_Orthographic:
		//meshData = generateOrthographicCameraModel(box);
		break;
	}

	std::shared_ptr<MeshBuffer> meshBuffer = std::make_shared<MeshBuffer>();
	code = allocateMeshBuffer(meshBuffer, meshData);

	if (code == ObjectAllocation::ReturnCode::Success)
		m_genericMeshes.insert({ id, meshBuffer });

	return code;
}

ObjectAllocation::ReturnCode MeshManager::allocateMeshBuffer(const std::shared_ptr<MeshBuffer>& meshBuffer, const RawMeshData& meshData)
{
	for (const std::pair<VkPrimitiveTopology, std::vector<StandardDraw>>& standardDrawPair : meshData.standardDraws)
		meshBuffer->addDraws(standardDrawPair.first, standardDrawPair.second);

	for (const std::pair<VkPrimitiveTopology, std::vector<IndexedDraw>>& indexedDrawPair : meshData.indexedDraws)
		meshBuffer->addIndexedDraws(indexedDrawPair.first, indexedDrawPair.second);

	VkResult vkRes = uploadInMeshBuffer(*meshBuffer, meshData.vertices, meshData.normals, meshData.indices);
	return ObjectAllocation::getVulkanReturnCode(vkRes);
}


RawMeshData MeshManager::generateBox()
{
	RawMeshData meshData;

	meshData.vertices = {
		{ -1.f, -1.f, -1.f },
		{ -1.f, -1.f,  1.f },
		{ -1.f,  1.f, -1.f },
		{ -1.f,  1.f,  1.f },
		{  1.f, -1.f, -1.f },
		{  1.f, -1.f,  1.f },
		{  1.f,  1.f, -1.f },
		{  1.f,  1.f,  1.f },
	};

	// cube as a single triangle strip (14 indices)
	meshData.indices = {
		// triangle strip (faces)
		0, 4, 5, 6, 7, 3, 5, 1, 0, 3, 2, 6, 0, 4,
		// line strip (edges)
		0, 1, 3, 2,
		3, 7, 5, 1,
		7, 6, 4, 5,
		4, 0, 2, 6
	};

	meshData.addIndexedDraw(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, { 0, 0, 14 });

	std::vector<IndexedDraw> edgeDrawList = {
		{ 0, 14, 4 },
		{ 0, 18, 4 },
		{ 0, 22, 4 },
		{ 0, 26, 4 }
	};
	meshData.addIndexedDraws(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, edgeDrawList);

	return meshData;
}

RawMeshData MeshManager::generateCylinder(const uint32_t& pointsInCircle)
{
	const glm::vec3 normBottom(0.0f, 0.0f, -1.0f), normTop(0.0f, 0.0f, 1.0f);

	RawMeshData meshData;

	const float angle(glm::pi<float>() * 2.0f / (float)pointsInCircle);
	uint32_t fan_size = pointsInCircle + 2;
	uint32_t strip_size = 2 * (pointsInCircle + 1);

	// Top triangle fan
	meshData.vertices.push_back(glm::vec3(0.0f, 0.0f, 1.0f));
	meshData.normals.push_back(normTop);
	for (uint32_t i = 0; i < pointsInCircle; ++i)
	{
		float x = sinf(-1 * angle * i);
		float y = cosf(angle * i);
		meshData.vertices.push_back(glm::vec3(x, y, 1.f));
		meshData.normals.push_back(normTop);
	}
	meshData.vertices.push_back(glm::vec3(0.0f, 1.0f, 1.0f));
	meshData.normals.push_back(normTop);
	assert(meshData.vertices.size() == fan_size && meshData.normals.size() == fan_size);
	meshData.addDraw(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, { 0, fan_size });
	meshData.addDraw(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, { 1, pointsInCircle + 1 });

	// Bottom triangle fan
	meshData.vertices.push_back(glm::vec3(0.0f, 0.0f, -1.0f));
	meshData.normals.push_back(normBottom);
	for (uint32_t i = 0; i < pointsInCircle; ++i)
	{
		float x = sinf(angle * i);
		float y = cosf(angle * i);
		meshData.vertices.push_back(glm::vec3(x, y, -1.f));
		meshData.normals.push_back(normBottom);
	}
	meshData.vertices.push_back(glm::vec3(0.0f, 1.0f, -1.0f));
	meshData.normals.push_back(normBottom);
	meshData.addDraw(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, { fan_size, fan_size });
	meshData.addDraw(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, { fan_size + 1, pointsInCircle + 1 });

	// Axis line
	meshData.indices = { 0, fan_size };
	meshData.addIndexedDraw(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, { 0, 0, 2 });

	for (uint32_t i = 0; i < pointsInCircle; ++i)
	{
		float x = sinf(angle * i);
		float y = cosf(angle * i);
		glm::vec3 top(x, y, 1.0f);
		glm::vec3 bottom(x, y, -1.0f);
		glm::vec3 normal(x, y, 0.0f);

		meshData.vertices.push_back(bottom);
		meshData.vertices.push_back(top);
		meshData.normals.push_back(normal);
		meshData.normals.push_back(normal);
	}
	meshData.vertices.push_back(glm::vec3(0.f, 1.f, -1.f));
	meshData.vertices.push_back(glm::vec3(0.f, 1.f, 1.f));
	meshData.normals.push_back(glm::vec3(0.f, 1.f, 0.f));
	meshData.normals.push_back(glm::vec3(0.f, 1.f, 0.f));
	meshData.addDraw(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, { 2 * fan_size, strip_size });

	return meshData;
}


RawMeshData MeshManager::generateSphere(const uint32_t& sectorCount, const uint32_t& stackCount)
{
	assert(stackCount >= 2 && sectorCount >= 3);
	const float m_pi2(glm::pi<float>() / 2.0f);
	const float m_2pi(glm::pi<float>() * 2.0f);
	const float sectorStep = m_2pi / sectorCount;
	const float stackStep = glm::pi<float>() / stackCount;

	RawMeshData meshData;

	meshData.vertices.reserve(stackCount * sectorCount * 2);
	meshData.normals.reserve(stackCount * sectorCount * 2);
	meshData.indices.reserve(stackCount * sectorCount * 4);

	// Pole nord
	meshData.vertices.push_back(glm::vec3(0, 0, 1.f));
	meshData.normals.push_back(glm::vec3(0, 0, 1.f));
	for (uint32_t j = 0; j < sectorCount; ++j)
	{
		const float sectorAngle = j * sectorStep;
		const float X = cosf(sectorAngle);
		const float Y = sinf(sectorAngle);
		for (uint32_t i = 1; i < stackCount; ++i)
		{
			const float stackAngle = m_pi2 - i * stackStep;
			const float x = X * cosf(stackAngle);
			const float y = Y * cosf(stackAngle);
			const float z = sinf(stackAngle);
			meshData.vertices.push_back({ x, y, z });
			meshData.normals.push_back({ x, y, z });
		}
	}
	// Pole sud
	meshData.vertices.push_back(glm::vec3(0, 0, -1.f));
	meshData.normals.push_back(glm::vec3(0, 0, -1.f));

	// 1 Triangle strip by sector
	for (uint32_t j = 0; j < sectorCount; ++j)
	{
		meshData.addIndexedDraw(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, { 0, (uint32_t)meshData.indices.size(), 2 * stackCount });
		meshData.indices.push_back(0);
		uint32_t k1 = j * (stackCount - 1);
		uint32_t k2 = ((j + 1) % sectorCount) * (stackCount - 1);
		for (uint32_t i = 1; i < stackCount; ++i)
		{
			meshData.indices.push_back(k1 + i);
			meshData.indices.push_back(k2 + i);
		}
		meshData.indices.push_back((uint32_t)meshData.vertices.size() - 1);
	}

	// Stack edges
	// NOTE - We recycle the indices list by usint a "vertex offset" for each stack edges
	uint32_t startIndex = (uint32_t)meshData.indices.size();
	for (uint32_t j = 0; j <= sectorCount; ++j)
		meshData.indices.push_back(1 + (j % sectorCount) * (stackCount - 1));
	for (uint32_t i = 0; i < stackCount - 1; ++i)
		meshData.addIndexedDraw(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, { (int32_t)i, startIndex, sectorCount + 1 });
	// Sector edges
	for (uint32_t j = 0; j < sectorCount; ++j)
	{
		uint32_t firstIndex = (uint32_t)meshData.indices.size();
		meshData.indices.push_back(0);
		uint32_t k1 = j * (stackCount - 1);
		for (uint32_t i = 1; i < stackCount; ++i)
		{
			meshData.indices.push_back(k1 + i);
		}
		meshData.indices.push_back((uint32_t)meshData.vertices.size() - 1);
		meshData.addIndexedDraw(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, { 0, firstIndex, stackCount + 1 });
	}

	return meshData;
}

RawMeshData MeshManager::generateTorusPart(uint32_t sectorCount, float angleRad, float ratioInnerOuterRadius)
{
	const float m_pi2(glm::pi<float>() / 2.0f);
	const float m_2pi(glm::pi<float>() * 2.0f);
	// Calculate and cache counts of vertices and indices
	const uint32_t mainSectorCount = (uint32_t)std::roundf((float)sectorCount * angleRad / m_2pi);
	const uint32_t numVertices = (sectorCount + 2) * (mainSectorCount + 3);

	// outer radius = 1.f
	float tubeRadius = (1.f - ratioInnerOuterRadius) / 2.f;
	float mainRadius = (1.f + ratioInnerOuterRadius) / 2.f;

	RawMeshData meshData;

	meshData.vertices.reserve(numVertices);
	meshData.normals.reserve(numVertices);

	float mainAngleStep(angleRad / (float)mainSectorCount);
	float tubeAngleStep(m_2pi / (float)sectorCount);

	// Vertices
	for (uint32_t j = 0; j < mainSectorCount + 1; ++j)
	{
		const float sectorAngle = j * mainAngleStep;
		glm::vec3 sectorCenter(cosf(sectorAngle) * mainRadius, sinf(sectorAngle) * mainRadius, 0.f);
		for (uint32_t i = 0; i < sectorCount; ++i)
		{
			const float stackAngle = i * tubeAngleStep;
			const glm::vec3 stackDir(cosf(sectorAngle) * cosf(stackAngle), sinf(sectorAngle) * cosf(stackAngle), sinf(stackAngle));

			meshData.vertices.push_back(sectorCenter + stackDir * tubeRadius);
			meshData.normals.push_back(stackDir);
		}
	}

	// 1 Triangle strip by section, but uses the same index order
	uint32_t k1 = 0;
	uint32_t k2 = sectorCount;
	for (uint32_t i = 0; i < sectorCount; ++i)
	{
		meshData.indices.push_back(k1 + i);
		meshData.indices.push_back(k2 + i);
	}
	meshData.indices.push_back(k1);
	meshData.indices.push_back(k2);
	// Section Faces
	for (uint32_t j = 0; j < mainSectorCount; ++j)
	{
		meshData.addIndexedDraw(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, { (int32_t)(j * sectorCount), 0, 2 * (sectorCount + 1) });
	}

	// Section faces
	glm::vec3 firstCenter(mainRadius, 0.f, 0.f);
	glm::vec3 firstNormal(0.f, -1.f, 0.f);
	uint32_t firstFaceVertex = (uint32_t)meshData.vertices.size();
	meshData.vertices.push_back(firstCenter);
	meshData.normals.push_back(firstNormal);
	for (uint32_t i = 0; i < sectorCount + 1; ++i)
	{
		const float stackAngle = (i % sectorCount) * tubeAngleStep;
		const glm::vec3 dir(cosf(stackAngle), 0.f, sinf(stackAngle));
		meshData.vertices.push_back(firstCenter + dir * tubeRadius);
		meshData.normals.push_back(firstNormal);
	}
	meshData.addDraw(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, { firstFaceVertex, sectorCount + 2 });
	meshData.addDraw(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, { firstFaceVertex + 1, sectorCount + 1 });

	glm::vec3 secondCenter(cosf(angleRad) * mainRadius, sinf(angleRad) * mainRadius, 0.f);
	glm::vec3 secondNormal(sinf(angleRad), -cosf(angleRad), 0.f);
	uint32_t secondFaceVertex = (uint32_t)meshData.vertices.size();
	meshData.vertices.push_back(secondCenter);
	meshData.normals.push_back(secondNormal);
	for (uint32_t i = 0; i < sectorCount + 1; ++i)
	{
		const float stackAngle = (i % sectorCount) * tubeAngleStep;
		const glm::vec3 dir(cosf(angleRad) * cosf(stackAngle), sinf(angleRad) * cosf(stackAngle), sinf(-stackAngle));
		meshData.vertices.push_back(secondCenter + dir * tubeRadius);
		meshData.normals.push_back(secondNormal);
	}
	meshData.addDraw(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN, { secondFaceVertex, sectorCount + 2 });
	meshData.addDraw(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, { secondFaceVertex + 1, sectorCount + 1 });

	// Central axis
	uint32_t firstAxisVertex = (uint32_t)meshData.vertices.size();
	for (uint32_t j = 0; j < mainSectorCount + 1; ++j)
	{
		const float sectorAngle = j * mainAngleStep;
		meshData.vertices.push_back(glm::vec3(cosf(sectorAngle) * mainRadius, sinf(sectorAngle) * mainRadius, 0.f));
	}
	meshData.addDraw(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP, { firstAxisVertex, mainSectorCount + 1 });

	// DONE
	return meshData;
}


RawMeshData MeshManager::generateBasicCameraModel()
{
	RawMeshData meshData;

	meshData.vertices = {
		{0.0f, 0.0f, 0.0},   //0
		{-0.5f, -0.36f, 0.5},//1
		{-0.5f, 0.36f, 0.5}, //2
		{0.5f, -0.36f, 0.5}, //3
		{0.5f, 0.36f, 0.5},  //4
		{-0.1f, 0.36f, 0.5}, //5
		{0.0f, 0.1f, 0.5},   //6
		{0.1f, 0.36f, 0.5}   //7
	};

	meshData.indices = {
		0, 1,
		0, 2,
		0, 3,
		0, 4,
		1, 2,
		1, 3,
		3, 4,
		2, 5,
		5, 6,
		6, 7,
		4, 7
	};
	meshData.addIndexedDraw(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, { 0, 0, (uint32_t)meshData.indices.size() });

	return meshData;
}

RawMeshData MeshManager::generatePerspectiveCameraModel(const ProjectionFrustum& box)
{
	RawMeshData meshData;

	float coef((float)(box.f / box.n));
	meshData.vertices = {
		 {box.l, box.b, box.n} //0
		,{box.l, box.t, box.n} //1
		,{box.r, box.b, box.n} //2
		,{box.r, box.t, box.n} //3
		,{box.l * coef, box.b * coef, box.f} //4
		,{box.l * coef, box.t * coef, box.f} //5
		,{box.r * coef, box.b * coef, box.f} //6
		,{box.r * coef, box.t * coef, box.f} //7
	};

	meshData.indices = {
		0, 1,
		0, 2,
		2, 3,
		3, 1,
		2, 1,
		0, 3,
		4, 5,
		4, 6,
		6, 7,
		7, 5,
		0, 4,
		1, 5,
		2, 6,
		3, 7
	};
	meshData.addIndexedDraw(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, { 0, 0, (uint32_t)meshData.indices.size() });

	return meshData;
}

RawMeshData MeshManager::generateOrthographicCameraModel(const ProjectionFrustum& box)
{
	RawMeshData meshData;

	meshData.vertices = {
		 {box.l, box.b, 0.0f} //0
		,{box.l, box.t, 0.0f} //1
		,{box.r, box.b, 0.0f} //2
		,{box.r, box.t, 0.0f} //3
		,{0.0f, 0.0f, 0.0f} //4
		,{0.0f, 0.0f, 1.0f} //5
	};

	meshData.indices = {
		0, 1,
		0, 2,
		2, 3,
		3, 1,
		2, 1,
		0, 3,
		4, 5,
	};
	meshData.addIndexedDraw(VK_PRIMITIVE_TOPOLOGY_LINE_LIST, { 0, 0, (uint32_t)meshData.indices.size() });

	return meshData;
}

VkResult MeshManager::uploadInMeshBuffer(MeshBuffer& meshBuffer, const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals, const std::vector<uint32_t>& indices)
{
	VulkanManager& vkm = VulkanManager::getInstance();
	SimpleBuffer& smpBuf = meshBuffer.m_smpBuffer;
	if (smpBuf.alloc != VK_NULL_HANDLE)
		vkm.freeAllocation(smpBuf);

	VkDeviceSize size = ((vertices.size() + normals.size()) * sizeof(glm::vec3)) + (indices.size() * sizeof(uint32_t));

	VkResult err = vkm.allocSimpleBuffer(size, smpBuf, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
	if (err)
	{
		GRAPH_LOG << "Error failed to allocate simple buffer of size" << size << Logger::endl;
		return err;
	}

	VkDeviceSize lastOffset = 0;
	meshBuffer.m_vertexBufferOffset = lastOffset;
	vkm.loadInSimpleBuffer(smpBuf, vertices.size() * sizeof(glm::vec3), vertices.data(), meshBuffer.m_vertexBufferOffset, 4);
	lastOffset = meshBuffer.m_vertexBufferOffset + vertices.size() * sizeof(glm::vec3);

	if (normals.size() > 0)
	{
		meshBuffer.m_normalBufferOffset = lastOffset;
		vkm.loadInSimpleBuffer(smpBuf, normals.size() * sizeof(glm::vec3), normals.data(), meshBuffer.m_normalBufferOffset, 4);
		lastOffset = meshBuffer.m_normalBufferOffset + normals.size() * sizeof(glm::vec3);
	}
	else
		meshBuffer.m_normalBufferOffset = 0;

	if (indices.size() > 0)
	{
		meshBuffer.m_indexBufferOffset = lastOffset;
		// draw count in the m_drawList
		vkm.loadInSimpleBuffer(smpBuf, indices.size() * sizeof(uint32_t), indices.data(), meshBuffer.m_indexBufferOffset, 4);
	}
	else
		meshBuffer.m_indexBufferOffset = 0;

	return err;
}

bool MeshManager::removeGenericMeshInstance(GenericMeshId id)
{
	if (m_simpleMeshCounters.find(id) == m_simpleMeshCounters.end())
	{
		assert(false);
		return false;
	}

	if (m_simpleMeshCounters.at(id) == 1)
	{
		removeGenericMesh(id);
		m_simpleMeshCounters.erase(id);
	}
	else
		m_simpleMeshCounters.at(id)--;
	return true;
}

bool MeshManager::removeMeshInstance(MeshId id)
{
	if (!id.isValid())
		return false;
	if (m_meshesCounters.find(id) == m_meshesCounters.end())
	{
		assert(false);
		return false;
	}

	if (m_meshesCounters.at(id) == 1)
	{
		removeMesh(id);
		m_meshesCounters.erase(id);
	}
	else
		m_meshesCounters.at(id)--;
	return true;
}

void MeshManager::removeGenericMesh(GenericMeshId id)
{
	std::lock_guard<std::mutex> lock(m_trashMutex);

	if (m_genericMeshes.find(id) != m_genericMeshes.end())
	{
		m_meshTrash.push_back(m_genericMeshes.at(id));
		m_genericMeshes.erase(id);
	}
}

void MeshManager::removeMesh(MeshId id)
{
	std::lock_guard<std::mutex> lock(m_trashMutex);

	if (m_meshes.find(id) != m_meshes.end())
	{
		SMesh meshToRemove = m_meshes.at(id);
		m_meshTrash.push_back(meshToRemove.m_mesh);

		std::list<size_t> toDeleteMLoaded;

		for(auto loaded = m_loaded.begin(); loaded != m_loaded.end(); loaded++)
		{
			if ((*loaded).second.meshIdInfo.find(id) != (*loaded).second.meshIdInfo.end())
			{
				loaded->second.meshIdInfo.erase(id);
				if (loaded->second.meshIdInfo.empty())
					toDeleteMLoaded.push_back(loaded->first);
			}
		}

		for (size_t toDelete : toDeleteMLoaded)
			m_loaded.erase(toDelete);

		m_meshes.erase(id);
	}
}

void MeshManager::removeGridMesh(const std::shared_ptr<MeshBuffer>& meshBuff)
{
	std::lock_guard<std::mutex> lock(m_trashMutex);
	if (meshBuff)
		m_meshTrash.push_back(meshBuff);
}

void MeshManager::emptyTrash()
{
	std::lock_guard<std::mutex> lock(m_trashMutex);

	for (std::shared_ptr<MeshBuffer> mesh : m_meshTrash)
	{
		VulkanManager::getInstance().freeAllocation(mesh->m_smpBuffer);
	}

	m_meshTrash.clear();
}

// L.1722