#include "controller/functionSystem/ContextExportFbx.h"
#include "controller/Controller.h"
#include "controller/ControllerContext.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "gui/Texts.hpp"
#include "gui/GuiData/GuiDataIO.h"

#include "controller/messages/DataIdListMessage.h"
#include "controller/messages/FilesMessage.h"
#include "controller/messages/PrimitivesExportParametersMessage.h"

#include "utils/Utils.h"
#include "utils/math/trigo.h"

#include "models/graph/MeshObjectNode.h"
#include "models/graph/CylinderNode.h"
#include "models/graph/SphereNode.h"
#include "models/graph/TorusNode.h"
#include "models/graph/BoxNode.h"

#include "models/graph/PipeToPipeMeasureNode.h"
#include "models/graph/PipeToPlaneMeasureNode.h"
#include "models/graph/PointToPipeMeasureNode.h"
#include "models/graph/PointToPlaneMeasureNode.h"
#include "models/graph/SimpleMeasureNode.h"
#include "models/graph/PolylineMeasureNode.h"

#include "vulkan/MeshManager.h"

#include <fbxsdk/utils/fbxgeometryconverter.h>
#include <fbxsdk/fileio/fbxiosettings.h>

#include <magic_enum/magic_enum.hpp>

ContextExportFbx::ContextExportFbx(const ContextId& id)
	: AContext(id)
{
	m_state = ContextState::waiting_for_input;
}

ContextExportFbx::~ContextExportFbx()
{}

ContextState ContextExportFbx::start(Controller& controller)
{
	return m_state;
}

ContextState ContextExportFbx::feedMessage(IMessage* message, Controller& controller)
{
	switch (message->getType())
	{
	case IMessage::MessageType::FILES:
		m_output = *(static_cast<FilesMessage*>(message)->m_inputFiles.begin());
		break;
	case IMessage::MessageType::DATAID_LIST:
		m_listToExport = static_cast<DataListMessage*>(message)->m_dataPtrs;
		if (m_listToExport.empty())
			return m_state = ContextState::abort;
		break;
	case IMessage::MessageType::PRIMITIVES_EXPORT_PARAMETERS:
		m_primitiveExportParam = static_cast<PrimitivesExportParametersMessage*>(message)->m_parameters;
		break;
	}

	m_state = (m_listToExport.empty() || m_output.empty()) ? ContextState::waiting_for_input : ContextState::ready_for_using;
	return m_state;
}

ContextState ContextExportFbx::launch(Controller& controller)
{
	m_state = ContextState::running;

	MeshManager& meshManager = MeshManager::getInstance();

	if (m_output.empty())
		return processError(controller);

	if (m_output.filename().wstring().find_first_of(L".") == std::wstring::npos)
		m_output += ".fbx";

	FbxManager* pManager = FbxManager::Create();
	if (!pManager)
		return processError(controller);

	pManager->SetIOSettings(FbxIOSettings::Create(pManager, IOSN_EXPORT));
	FbxIOSettings& IOS = *(pManager->GetIOSettings());

	std::string sceneName = Utils::to_utf8(m_output.filename().stem().wstring());
	//Create an FBX scene. This object holds most objects imported/exported from/to files.
	FbxScene* pScene = FbxScene::Create(pManager, sceneName.c_str());
	if (!pScene)
		return processError(controller);

	pScene->GetGlobalSettings().SetSystemUnit(fbxsdk::FbxSystemUnit(100));

	std::unordered_map<ElementType, FbxMesh*> meshMap;

	for (const SafePtr<AGraphNode>& data : m_listToExport)
	{
		ElementType exportType;
		glm::dmat4 transfo = glm::dmat4();
		Color32 color;
		std::wstring name;
		{
			ReadPtr<AGraphNode> rData = data.cget();
			if (!rData)
				continue;
			exportType = rData->getType();
			transfo = rData->getTransformation();
			name = rData->getName();
			color = rData->getColor();
		}

		FbxMesh* currentMesh = nullptr;
		std::vector<std::vector<glm::dvec3>> measures = {};
		std::vector<Measure> simpleMeasures = {};

		switch (exportType)
		{
		case ElementType::Cylinder:
		{
			if (meshMap.find(exportType) == meshMap.end())
			{
				std::pair<ElementType, FbxMesh*> pair({ exportType, nullptr });
				pair.second = createMesh(pScene, Utils::from_utf8(std::string(magic_enum::enum_name(exportType))), meshManager.generateCylinder(48));
				meshMap.insert(pair);
			}
			currentMesh = meshMap.at(exportType);
		}
		break;
		case ElementType::Sphere:
		{ 
			if (meshMap.find(exportType) == meshMap.end())
			{
				std::pair<ElementType, FbxMesh*> pair({ exportType, nullptr });
				pair.second = createMesh(pScene, Utils::from_utf8(std::string(magic_enum::enum_name(exportType))), meshManager.generateSphere(48, 48));
				meshMap.insert(pair);
			}
			currentMesh = meshMap.at(exportType);
		}
		break;
		case ElementType::Box:
		case ElementType::Grid:
		{
			if (meshMap.find(exportType) == meshMap.end())
			{
				std::pair<ElementType, FbxMesh*> pair({ exportType, nullptr });
				pair.second = createMesh(pScene, Utils::from_utf8(std::string(magic_enum::enum_name(exportType))), meshManager.generateBox());
				meshMap.insert(pair);
			}
			currentMesh = meshMap.at(exportType);
		}
		break;
		case ElementType::Torus:
		{
			ReadPtr<TorusNode> rTorus = static_pointer_cast<TorusNode>(data).cget();
			if (!rTorus)
				continue;
			float outerRadius = rTorus->getMainRadius() + rTorus->getAdjustedTubeRadius();
			float innerRadius = rTorus->getMainRadius() - rTorus->getAdjustedTubeRadius();

			currentMesh = createMesh(pScene, name + L"_mesh", meshManager.generateTorusPart(48, rTorus->getMainAngle(), innerRadius / outerRadius));
		}
		break;
		case ElementType::MeshObject:
		{
			ReadPtr<MeshObjectNode> rMesh = static_pointer_cast<MeshObjectNode>(data).cget();
			if (!rMesh)
				continue;
			std::shared_ptr<MeshBuffer> buffer = meshManager.getMesh(rMesh->getMeshId()).m_mesh;

			RawMeshData meshData;
			meshBufferToRawMeshData(buffer, meshData);
			currentMesh = createMesh(pScene, name + L"_mesh", meshData);
		}
		break;
		case ElementType::PipeToPipeMeasure:
		case ElementType::PipeToPlaneMeasure:
		case ElementType::PointToPipeMeasure:
		case ElementType::PointToPlaneMeasure:
		case ElementType::SimpleMeasure:
		{
			ReadPtr<AMeasureNode> rMeasures = static_pointer_cast<AMeasureNode>(data).cget();
			if (!rMeasures)
				continue;
			simpleMeasures = rMeasures->getMeasures();
		}
		break;
		case ElementType::PolylineMeasure:
		{
			ReadPtr<AMeasureNode> rMeasures = static_pointer_cast<AMeasureNode>(data).cget();
			if (!rMeasures)
				continue;
			std::vector<Measure> polylineMeasures = rMeasures->getMeasures();
			std::vector<glm::dvec3> polyline;
			if (polylineMeasures.empty())
				continue;
			polyline.push_back(polylineMeasures.begin()->origin);
			for (const Measure& m : polylineMeasures)
				polyline.push_back(m.final);
			measures.push_back(polyline);
		}
		break;
		default:
			continue;
		}

		glm::dvec3 decalExport = m_primitiveExportParam.exportWithScanImportTranslation ? -controller.getContext().getProjectInfo().m_importScanTranslation : glm::dvec3(0);
		glm::dmat4 decalMat = { 1.0f, 0.f, 0.f, 0.f,
							  0.f, 1.0f, 0.f, 0.f,
							  0.f, 0.f, 1.0f, 0.f,
							  decalExport.x, decalExport.y, decalExport.z, 1.f };
		transfo = decalMat * transfo;

		for (const Measure& m : simpleMeasures)
		{
			std::vector<glm::dvec3> line;
			line.push_back(m.origin);
			line.push_back(m.final);
			measures.push_back(line);
		}

		if (currentMesh)
			addObject(pScene, currentMesh, transfo, name, color);
		else if (!measures.empty())
			addObject(pScene, measures, transfo, name, color);


	}

	FbxExporter* lExporter = FbxExporter::Create(pManager, "");

	int pFileFormat = -1;
	if (pFileFormat < 0 || pFileFormat >= pManager->GetIOPluginRegistry()->GetWriterFormatCount())
	{
		// Write in fall back format in less no ASCII format found
		pFileFormat = pManager->GetIOPluginRegistry()->GetNativeWriterFormat();

		//Try to export in binary if possible
		int lFormatIndex, lFormatCount = pManager->GetIOPluginRegistry()->GetWriterFormatCount();

		for (lFormatIndex = 0; lFormatIndex < lFormatCount; lFormatIndex++)
		{
			if (pManager->GetIOPluginRegistry()->WriterIsFBX(lFormatIndex))
			{
				FbxString lDesc = pManager->GetIOPluginRegistry()->GetWriterFormatDescription(lFormatIndex);
				const char* lBinary = "binary";
				if (lDesc.Find(lBinary) >= 0)
				{
					pFileFormat = lFormatIndex;
					break;
				}
			}
		}
	}

	IOS.SetBoolProp(EXP_FBX_MATERIAL, false);
	IOS.SetBoolProp(EXP_FBX_TEXTURE, false);
	IOS.SetBoolProp(EXP_FBX_EMBEDDED, false);
	IOS.SetBoolProp(EXP_FBX_SHAPE, true);
	IOS.SetBoolProp(EXP_FBX_GOBO, false);
	IOS.SetBoolProp(EXP_FBX_ANIMATION, false);
	IOS.SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

	if (!lExporter->Initialize(Utils::to_utf8(m_output.wstring()).c_str(), pFileFormat, pManager->GetIOSettings()))
	{
		pManager->Destroy();
		return processError(controller);
	}

	if (!lExporter->Export(pScene))
	{
		pManager->Destroy();
		return processError(controller);
	}

	if (m_primitiveExportParam.openFolderWindowsAfterExport)
		controller.updateInfo(new GuiDataOpenInExplorer(m_output));
	return (m_state = ContextState::done);
}

bool ContextExportFbx::canAutoRelaunch() const
{
	return (false);
}

ContextType ContextExportFbx::getType() const
{
	return (ContextType::exportFbx);
}

ContextState ContextExportFbx::processError(Controller& controller)
{
	controller.updateInfo(new GuiDataWarning(TEXT_WRITE_FAILED_PERMISSION));
	return (m_state = ContextState::abort);
}

FbxMesh* ContextExportFbx::createMesh(FbxScene* scene, const std::wstring& meshName, const RawMeshData& geom)
{
	FbxMesh* pMesh = FbxMesh::Create(scene, Utils::to_utf8(meshName).c_str());

	pMesh->InitControlPoints((int)geom.vertices.size());
	FbxVector4* controlPoints = pMesh->GetControlPoints();

	/*FbxGeometryElementNormal* normalLayer = nullptr;
	if (!geometrie.normals.empty() && geometrie.normals.size() == geometrie.vertices.size())
	{
		normalLayer = pMesh->CreateElementNormal();
		normalLayer->SetMappingMode(FbxGeometryElement::eByControlPoint);
		normalLayer->SetReferenceMode(FbxGeometryElement::eDirect);
	}*/

	for (int iVert = 0; iVert < geom.vertices.size(); iVert++)
	{
		FbxVector4 controlPoint(geom.vertices[iVert].x, geom.vertices[iVert].y, geom.vertices[iVert].z);
		controlPoints[iVert] = controlPoint;
		/*if (normalLayer)
			normalLayer->GetDirectArray().Add(FbxVector4(geometrie.normals[iVert], geometrie.normals[iVert + 1], geometrie.normals[iVert + 2]));*/
	}

	std::vector<uint32_t> indexes;
	getTrianglesList(geom, indexes);
	assert((indexes.size() % 3) == 0);

	for (auto ind = indexes.begin(); ind != indexes.end(); ind += 3)
	{
		pMesh->BeginPolygon();
		pMesh->AddPolygon(*ind);
		pMesh->AddPolygon(*(ind + 1));
		pMesh->AddPolygon(*(ind + 2));
		pMesh->EndPolygon();
	}

	pMesh->BeginAddMeshEdgeIndex();

	if (geom.indexedDraws.find(VK_PRIMITIVE_TOPOLOGY_LINE_LIST) != geom.indexedDraws.end())
	{
		for (const IndexedDraw& indDraw : geom.indexedDraws.at(VK_PRIMITIVE_TOPOLOGY_LINE_LIST))
			for (uint32_t i = indDraw.firstIndex; i < indDraw.firstIndex + indDraw.indexCount - 1; i += 2)
				pMesh->AddMeshEdgeIndex(geom.indices[i], geom.indices[i + 1], false);
	}

	if (geom.indexedDraws.find(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP) != geom.indexedDraws.end())
	{
		for (const IndexedDraw& indDraw : geom.indexedDraws.at(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP))
			for (uint32_t i = indDraw.firstIndex; i < indDraw.firstIndex + indDraw.indexCount - 1; i++)
				pMesh->AddMeshEdgeIndex(geom.indices[i], geom.indices[i + 1], false);
	}

	if (geom.standardDraws.find(VK_PRIMITIVE_TOPOLOGY_LINE_LIST) != geom.standardDraws.end())
	{
		for (const StandardDraw& stdDraw : geom.standardDraws.at(VK_PRIMITIVE_TOPOLOGY_LINE_LIST))
			for (uint32_t i = stdDraw.firstVertex; i < stdDraw.firstVertex + stdDraw.vertexCount - 1; i += 2)
				pMesh->AddMeshEdgeIndex(i, i + 1, false);
	}

	if (geom.standardDraws.find(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP) != geom.standardDraws.end())
	{
		for (const StandardDraw& stdDraw : geom.standardDraws.at(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP))
			for (uint32_t i = stdDraw.firstVertex; i < stdDraw.firstVertex + stdDraw.vertexCount - 1; i++)
				pMesh->AddMeshEdgeIndex(i, i + 1, false);
	}

	pMesh->EndAddMeshEdgeIndex();

	return pMesh;
}

void ContextExportFbx::addObject(FbxScene* scene, FbxMesh* pMesh, const glm::mat4& transfo, const std::wstring& name, const Color32& color)
{
	FbxLayer* lLayer = pMesh->GetLayer(0);
	if (!lLayer)
		return;

	glm::dvec3 tr;
	glm::dquat rot;
	glm::dvec3 sc;
	TransformationModule::getTrRtSc(transfo, tr, rot, sc);

	glm::dvec3 rotDeg = tls::math::quat_to_euler_zyx_deg(rot);

	FbxLayerElementMaterial* lLayerElementMaterial = FbxLayerElementMaterial::Create(pMesh, "colorLayer");
	lLayerElementMaterial->SetMappingMode(FbxLayerElement::eAllSame);
	lLayerElementMaterial->SetReferenceMode(FbxLayerElement::eIndexToDirect);
	lLayerElementMaterial->GetIndexArray().Add(0);

	lLayer->SetMaterials(lLayerElementMaterial);


	FbxSurfacePhong* lMaterial = FbxSurfacePhong::Create(scene, "color");
	FbxDouble3 fbxColor = FbxDouble3((double)color.r / 255., (double)color.g / 255., (double)color.b / 255.);

	lMaterial->Diffuse.Set(fbxColor);
	lMaterial->DiffuseFactor.Set(1.);
	lMaterial->ShadingModel.Set("phong");
	lMaterial->Shininess.Set(0.5);
	lMaterial->Specular.Set(FbxDouble3(0.0, 0.0, 0.0));
	lMaterial->SpecularFactor.Set(0.3);

	FbxNode* lNode = FbxNode::Create(scene, Utils::to_utf8(name).c_str());
	lNode->SetNodeAttribute(pMesh);
	lNode->AddMaterial(lMaterial);

	lNode->LclTranslation.Set(FbxVector4(tr.x, tr.y, tr.z, 1.0f));
	lNode->LclRotation.Set(FbxVector4(rotDeg.x, rotDeg.y, rotDeg.z, 1.0f));
	lNode->LclScaling.Set(FbxVector4(sc.x, sc.y, sc.z, 1.0f));

	FbxNode* lRootNode = scene->GetRootNode();
	lRootNode->AddChild(lNode);
}

void ContextExportFbx::addObject(FbxScene* scene, const std::vector<std::vector<glm::dvec3>>& mesures, const glm::mat4& transfo, const std::wstring& name, const Color32& color)
{
	FbxLine* fbxline = FbxLine::Create(scene, Utils::to_utf8(name).c_str());

	uint32_t size = 0;
	for (const std::vector<glm::dvec3>& line : mesures)
		size += (uint32_t)line.size();

	fbxline->InitControlPoints(size);
	FbxVector4* controlPoints = fbxline->GetControlPoints();

	uint32_t iPoint = 0;
	for(const std::vector<glm::dvec3>& line : mesures)
		for (uint32_t pointInd = 0; pointInd < line.size(); pointInd++)
		{
			glm::dvec3 point = line[pointInd];
			point = transfo * glm::dvec4(point, 1.0);
			controlPoints[iPoint] = FbxVector4(point.x, point.y, point.z, 1.0);
			fbxline->AddPointIndex(iPoint, pointInd == line.size() - 1);
			iPoint++;
		}

	FbxNode* lNode = FbxNode::Create(scene, Utils::to_utf8(name).c_str());
	lNode->SetNodeAttribute(fbxline);

	FbxNode* lRootNode = scene->GetRootNode();
	lRootNode->AddChild(lNode);
}

void ContextExportFbx::meshBufferToRawMeshData(const std::shared_ptr<MeshBuffer>& meshBuffer, RawMeshData& meshData)
{
	if (meshBuffer == nullptr)
		return;

	size_t bufferSize = meshBuffer->getSimpleBuffer().size;
	void* bufferData = new char[bufferSize];
	memset(bufferData, 0, bufferSize);

	VulkanManager::getInstance().downloadSimpleBuffer(meshBuffer->getSimpleBuffer(), bufferData, bufferSize);

	glm::vec3* pointsData = static_cast<glm::vec3*>(bufferData);
	size_t vCount = meshBuffer->getVertexCount();
	meshData.vertices.reserve(vCount);
	for (size_t i = 0; i < vCount; ++i)
		meshData.vertices.emplace_back(glm::vec4(pointsData[i], 1.f));

	getRawIndicesData(bufferData, meshBuffer, meshData.indices);
	delete[] bufferData;

	meshData.indexedDraws = meshBuffer->getIndexedDrawList();
	meshData.standardDraws = meshBuffer->getDrawList();
}

void ContextExportFbx::getRawIndicesData(void* bufferData, const std::shared_ptr<MeshBuffer>& meshBuffer, std::vector<uint32_t>& indices)
{
	uint32_t indicesCount = 0;
	for (auto topoIDraw : meshBuffer->getIndexedDrawList())
	{
		for (const IndexedDraw& idd : topoIDraw.second)
			indicesCount += idd.indexCount;
	}
	indices.reserve(indicesCount);

	uint32_t* inds = static_cast<uint32_t*>((void*)((char*)bufferData + meshBuffer->getIndexOffset()));

	indices.insert(indices.begin(), inds, inds + indicesCount);
}


void ContextExportFbx::getTrianglesList(const RawMeshData& meshData, std::vector<uint32_t>& indexes)
{
	uint32_t triangleCount = 0;
	for (auto topoDraw : meshData.standardDraws)
	{
		for (const StandardDraw& stdDraw : topoDraw.second)
		{
			if (topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
				triangleCount += stdDraw.vertexCount / 3;
			if (topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP ||
				topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
				triangleCount += stdDraw.vertexCount - 2;
		}
	}

	for (auto topoIDraw : meshData.indexedDraws)
	{
		for (const IndexedDraw& idd : topoIDraw.second)
		{
			if (topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
				triangleCount += idd.indexCount / 3;
			if (topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP ||
				topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
				triangleCount += idd.indexCount - 2;
		}
	}
	indexes.reserve(triangleCount);


	for (auto topoDraw : meshData.standardDraws)
	{
		for (const StandardDraw& stdDraw : topoDraw.second)
		{
			if (topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			{
				for (uint32_t i = stdDraw.firstVertex; i < stdDraw.firstVertex + stdDraw.vertexCount - 2; i += 3)
				{
					indexes.push_back(i);
					indexes.push_back(i + 1);
					indexes.push_back(i + 2);
				}
			}
			else if (topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP)
			{
				for (uint32_t i = stdDraw.firstVertex; i < stdDraw.firstVertex + stdDraw.vertexCount - 2; ++i)
				{
					indexes.push_back(i);
					indexes.push_back(i + (1 + i % 2));
					indexes.push_back(i + (2 - i % 2));
				}
			}
			else if (topoDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
			{
				for (uint32_t i = stdDraw.firstVertex; i < stdDraw.firstVertex + stdDraw.vertexCount - 2; ++i)
				{
					indexes.push_back(i + 1);
					indexes.push_back(i + 2);
					indexes.push_back(stdDraw.firstVertex);
				}
			}
		}
	}

	const std::vector<uint32_t>& inds = meshData.indices;

	for (auto topoIDraw : meshData.indexedDraws)
	{
		for (const IndexedDraw& idd : topoIDraw.second)
		{
			if (topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
			{
				for (uint32_t i = idd.firstIndex; i < idd.firstIndex + idd.indexCount - 2; i += 3)
				{
					indexes.push_back(idd.vertexOffset + inds[i]);
					indexes.push_back(idd.vertexOffset + inds[i + 1]);
					indexes.push_back(idd.vertexOffset + inds[i + 2]);
				}
			}
			else if (topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP)
			{
				for (uint32_t i = idd.firstIndex; i < idd.firstIndex + idd.indexCount - 2; ++i)
				{
					indexes.push_back(idd.vertexOffset + inds[i]);
					indexes.push_back(idd.vertexOffset + inds[i + (1 + i % 2)]);
					indexes.push_back(idd.vertexOffset + inds[i + (2 - i % 2)]);
				}
			}
			else if (topoIDraw.first == VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN)
			{
				for (uint32_t i = idd.firstIndex; i < idd.firstIndex + idd.indexCount - 2; ++i)
				{
					indexes.push_back(idd.vertexOffset + inds[i + 1]);
					indexes.push_back(idd.vertexOffset + inds[i + 2]);
					indexes.push_back(idd.vertexOffset + inds[0]);
				}
			}
		}
	}
}

