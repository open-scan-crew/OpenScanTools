#include "io\imports\fbxWriterReader.h"
#include "fbxsdk/utils/fbxgeometryconverter.h"
#include "utils/Logger.h"
#include "utils/Utils.h"
#include <unordered_set>

#include "controller/Controller.h"
#include "gui/GuiData/GuiDataMessages.h"
#include "controller/functionSystem/FunctionManager.h"
#include "gui/texts/SplashScreenTexts.hpp"

#include <map>


#define INTERN_FILE_GUID_V1 "73b1709e-5fd5-438c-b76e-6eab9c571a70"

fbxWriterReader::fbxWriterReader(Controller* pController, const MeshObjInputData& inputInfo)
	: IMeshReader(pController, inputInfo)
{
	m_manager = FbxManager::Create();
	m_scene = FbxScene::Create(m_manager, "My Scene");
	m_internLoadVersion = 0;
}

fbxWriterReader::~fbxWriterReader()
{
	if (m_scene)
		m_scene->Destroy(true);
	if (m_manager) 
		m_manager->Destroy();
}

bool fbxWriterReader::read()
{
	m_models.clear();
	if (!m_manager)
		return false;

	m_manager->SetIOSettings(FbxIOSettings::Create(m_manager, IOSROOT));
	FbxIOSettings& IOS = *(m_manager->GetIOSettings());
	
	if (!m_scene)
		return false;

	bool lResult;
	FbxString lFilePath(Utils::to_utf8(m_inputInfo.path.wstring()).c_str());

	if (lFilePath.IsEmpty())
		lResult = false;
	else
		lResult = LoadScene(m_manager, m_scene, lFilePath.Buffer());

	if (!lResult)
		return false;

	m_scale = m_scene->GetGlobalSettings().GetSystemUnit().GetScaleFactor()/100;

	FbxNode* rootNode = m_scene->GetRootNode();
	std::string name = m_scene->GetName();

	if (name == INTERN_FILE_GUID_V1)
		m_internLoadVersion = 1;
	else
		m_internLoadVersion = 0; //No intern fbx file load


	recGetChildMesh(rootNode);

	m_maxCount = (int)m_models.size();

	return true; 
}


void fbxWriterReader::recGetChildMesh(FbxNode* node)
{
	FbxNodeAttribute::EType lAttributeType;
	int i;

	if (node->GetNodeAttribute() != NULL)
	{
		lAttributeType = (node->GetNodeAttribute()->GetAttributeType());

		switch (lAttributeType)
		{
			case FbxNodeAttribute::eLine:
			case FbxNodeAttribute::eMesh:
			{
				m_models.push_back(node);
				break;
			}
			default:
				break;
		}
	}

	for (i = 0; i < node->GetChildCount(); i++)
		recGetChildMesh(node->GetChild(i));
}

bool fbxWriterReader::loadNode(FbxNode* node, MeshGeometries& geom)
{
	switch (node->GetNodeAttribute()->GetAttributeType())
	{
		case FbxNodeAttribute::eMesh:
			return loadMesh(node, geom);
		case FbxNodeAttribute::eLine:
			return loadLine(node, geom);
	}

	return false;
}

bool fbxWriterReader::loadMesh(FbxNode* node, MeshGeometries& geom)
{
	if (node->GetNodeAttribute()->GetAttributeType() != FbxNodeAttribute::eMesh)
		return false;

	FbxMesh* meshNode = (FbxMesh*)node->GetNodeAttribute();

	/*if (m_inputInfo.generateEdges && meshNode->GetMeshEdgeCount() <= 0)
		meshNode->BuildMeshEdgeArray();*/

	std::vector<uint32_t> tempEdgesInds;

	int startGeomVerticesSize = ((int)geom.vertices.size() / 3);

	meshNode->BeginGetMeshEdgeVertices();
	for (int i = 0; i < meshNode->GetMeshEdgeCount(); i++)
	{
		int a, b;
		meshNode->GetMeshEdgeVertices(i, a, b);
		if (a == -1 || b == -1)
			assert(false);

		geom.edgesIndices.push_back(startGeomVerticesSize + a);
		geom.edgesIndices.push_back(startGeomVerticesSize + b);
	}
	meshNode->EndGetMeshEdgeVertices();

	FbxGeometryConverter geomConvert(m_manager);
	if (!meshNode->IsTriangleMesh())
		meshNode = (FbxMesh*)geomConvert.Triangulate(meshNode, true, false);

	if (!meshNode->IsTriangleMesh())
		return false;

	FbxVector4* ctrlps = meshNode->GetControlPoints();
	FbxGeometryElementNormal* normals = nullptr;
	int normCount = meshNode->GetElementNormalCount();
	if (normCount != 0)
		normals = meshNode->GetElementNormal(0);

	bool hasNormals = normals && normals->GetMappingMode() == FbxGeometryElement::eByControlPoint;

	//std::map<int, uint32_t> ctrlpsIndToInd;

	geom.vertices.resize(3 * (size_t)(startGeomVerticesSize + meshNode->GetControlPointsCount()));
	if(hasNormals)
		geom.normals.resize(3 * (size_t)(startGeomVerticesSize + meshNode->GetControlPointsCount()));

	for (int i = 0; i < meshNode->GetPolygonCount(); i++)
	{
		assert(meshNode->GetPolygonSize(i) == 3);
		for (int j = 0; j < meshNode->GetPolygonSize(i); j++)
		{
			int verIndex = meshNode->GetPolygonVertex(i, j);
			std::array<float, 3> point = transformFbxVertex(node, ctrlps[verIndex], false);

			geom.vertices[(startGeomVerticesSize + verIndex) * 3] = point[0];
			geom.vertices[(startGeomVerticesSize + verIndex) * 3 + 1] = point[1];
			geom.vertices[(startGeomVerticesSize + verIndex) * 3 + 2] = point[2];

			geom.indices.push_back(startGeomVerticesSize + verIndex);

			//ctrlpsIndToInd[verIndex] = ind;

			if (hasNormals)
			{
				int normIndex = -1;
				if (normals->GetReferenceMode() == FbxGeometryElement::eDirect)
					normIndex = verIndex;
				if (normals->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
					normIndex = normals->GetIndexArray().GetAt(verIndex);

				if (normIndex >= 0)
				{
					FbxVector4 vec = normals->GetDirectArray().GetAt(normIndex);
					std::array<float, 3> normal = transformFbxVertex(node, vec, true);//{ (float)vec[0] , (float)vec[1] , (float)vec[2] };
					glm::vec3 gNormal = glm::normalize(glm::vec3(normal[0], normal[1], normal[2]));
					geom.normals[(startGeomVerticesSize + normIndex) * 3] = gNormal.x;
					geom.normals[(startGeomVerticesSize + normIndex) * 3 + 1] = gNormal.y;
					geom.normals[(startGeomVerticesSize + normIndex) * 3 + 2] = gNormal.z;
				}
			}
		}
	}

	/*for (int edgeInd : tempEdgesInds)
		geom.edgesIndices.push_back(ctrlpsIndToInd[edgeInd]);*/

	return true;
}

bool fbxWriterReader::loadLine(FbxNode* node, MeshGeometries& geom)
{
	if (node->GetNodeAttribute()->GetAttributeType() != FbxNodeAttribute::eLine)
		return false;

	FbxLine* lineNode = (FbxLine*)node->GetNodeAttribute();

	int startGeomVerticesSize = ((int)geom.vertices.size() / 3);

	FbxVector4* ctrlps = lineNode->GetControlPoints();
	for (int i = 0; i < lineNode->GetControlPointsCount(); i++)
	{
		std::array<float, 3> point = transformFbxVertex(node, ctrlps[i], false);

		geom.vertices.push_back(point[0]);
		geom.vertices.push_back(point[1]);
		geom.vertices.push_back(point[2]);
	}

	FbxArray<int>* pointsIndex = lineNode->GetIndexArray();
	FbxArray<int>* endIndex = lineNode->GetEndPointArray();

	std::vector<uint32_t> polyline;
	for (int i = 0; i < lineNode->GetIndexArraySize(); i++)
	{
		polyline.push_back(startGeomVerticesSize + (uint32_t)pointsIndex->GetAt(i));
		if (endIndex->Find(i, 0) != -1)
		{
			if (polyline.size() == 2)
			{
				geom.edgesIndices.push_back(polyline[0]);
				geom.edgesIndices.push_back(polyline[1]);
			}
			else if (polyline.size() > 2)
				geom.polyligneIndices.push_back(polyline);

			polyline.clear();
		}
	}

	return true;
}

std::array<float, 3> fbxWriterReader::transformFbxVertex(FbxNode* node, const FbxVector4& vertice, bool onlyRotation)
{
	FbxAMatrix matrixGeo;
	matrixGeo.SetIdentity();
	if (node->GetNodeAttribute())
	{
		const FbxVector4 lR = node->GetGeometricRotation(FbxNode::eSourcePivot);
		matrixGeo.SetR(lR);
		if (!onlyRotation)
		{
			const FbxVector4 lT = node->GetGeometricTranslation(FbxNode::eSourcePivot);
			const FbxVector4 lS = node->GetGeometricScaling(FbxNode::eSourcePivot);
			matrixGeo.SetT(lT);
			matrixGeo.SetS(lS);
		}
	}
	FbxAMatrix globalMatrix = node->EvaluateLocalTransform();

	FbxAMatrix matrix = globalMatrix * matrixGeo;
	FbxVector4 tVertice = matrix.MultT(vertice);

	return { (float)tVertice[0], (float)tVertice[1], (float)tVertice[2] };
}

// Note(Quentin) : Fonction spécifique pour générer les geométries à partir de nos fichiers internes de sauvegarde de mesh
ObjectAllocation::ReturnCode fbxWriterReader::generateInternalFilesGeometries()
{
	if (m_models.size() != 1)
		return ObjectAllocation::ReturnCode::Load_File_Error;

	MeshShape merge_meshShape;

	m_loadCount++;

	for (FbxNode* nodeToLoad : m_models)
	{
		if (!loadNode(nodeToLoad, merge_meshShape.geometry))
			continue;
	}

	if(merge_meshShape.geometry.vertices.empty())
		return ObjectAllocation::ReturnCode::Failed;

	m_meshesShapes.push_back(std::move(merge_meshShape));

	if (!updateImportProcessUI(TEXT_SPLASH_SCREEN_IMPORT_GEOMETRIES.arg(m_loadCount).arg(m_models.size()), true))
		return ObjectAllocation::ReturnCode::Aborted;


	m_scene->Destroy(true);
	m_scene = nullptr;

	m_manager->Destroy();
	m_manager = nullptr;

	return ObjectAllocation::ReturnCode::Success;
}

ObjectAllocation::ReturnCode fbxWriterReader::generateGeometries()
{
	if (m_internLoadVersion > 0)
		return generateInternalFilesGeometries();

	MeshShape merge_meshShape;
	merge_meshShape.name = Utils::from_utf8(m_scene->GetName());

	int addCount = (int)m_models.size() / 20;
	int count = 0;

	for (FbxNode* node : m_models)
	{
		count++;

		MeshShape meshShape;
		meshShape.name = Utils::from_utf8(node->GetName());

		if(!loadNode(node, m_inputInfo.isMerge ? merge_meshShape.geometry : meshShape.geometry))
			return ObjectAllocation::ReturnCode::Load_File_Error;

		if (addCount != 0 && count % addCount == 0)
		{
			m_loadCount += addCount;
			if (!updateImportProcessUI(TEXT_SPLASH_SCREEN_IMPORT_GEOMETRIES.arg(count).arg(m_models.size()), true))
				return ObjectAllocation::ReturnCode::Aborted;
		}

		if (!m_inputInfo.isMerge)
			m_meshesShapes.push_back(std::move(meshShape));
	}
	if(m_inputInfo.isMerge)
		m_meshesShapes.push_back(std::move(merge_meshShape));

	m_loadCount += m_models.size() % 20;
	if (!updateImportProcessUI(TEXT_SPLASH_SCREEN_IMPORT_GEOMETRIES.arg(count).arg(m_models.size()), true))
		return ObjectAllocation::ReturnCode::Aborted;

	m_scene->Destroy(true);
	m_scene = nullptr;

	m_manager->Destroy();
	m_manager = nullptr;

	return ObjectAllocation::ReturnCode::Success;
}

xg::Guid fbxWriterReader::getLoadedMeshId() const
{
	if (m_internLoadVersion == 1)
	{
		assert(m_models.size() == 1);
		std::string meshId = (*m_models.begin())->GetNodeAttribute()->GetName();
		xg::Guid internMeshId = xg::Guid(meshId);
		return internMeshId;
	}
	return xg::Guid();
}

bool fbxWriterReader::LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename)
{
	int lFileMajor, lFileMinor, lFileRevision;
	int lSDKMajor, lSDKMinor, lSDKRevision;
	//int lFileFormat = -1;
	//int lAnimStackCount;
	bool lStatus;
	//char lPassword[1024];

	FbxIOSettings& IOS = *(pManager->GetIOSettings());

	// Get the file version number generate by the FBX SDK.
	FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

	// Create an importer.
	FbxImporter* lImporter = FbxImporter::Create(pManager, "");

	// Initialize the importer by providing a filename.
	const bool lImportStatus = lImporter->Initialize(pFilename, -1, pManager->GetIOSettings());
	lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

	if (!lImportStatus)
	{
		FbxString error = lImporter->GetStatus().GetErrorString();

		if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
		{
			IOLOG << "FBX file format version for this FBX SDK is " << lSDKMajor << lSDKMinor << lSDKRevision << LOGENDL;
			IOLOG << "FBX file format version for file " << pFilename << lFileMajor << lFileMinor << lFileRevision << LOGENDL;
		}

		return false;
	}

	if (lImporter->IsFBX())
	{

		// Set the import states. By default, the import states are always set to 
		// true. The code below shows how to change these states.

		IOS.SetBoolProp(IMP_FBX_MATERIAL, false);
		IOS.SetBoolProp(IMP_FBX_TEXTURE, false);
		IOS.SetBoolProp(IMP_FBX_LINK, true);
		IOS.SetBoolProp(IMP_FBX_SHAPE, true);
		IOS.SetBoolProp(IMP_FBX_NORMAL, true);
		IOS.SetBoolProp(IMP_FBX_GOBO, true);
		IOS.SetBoolProp(IMP_FBX_ANIMATION, false);
		IOS.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
	}

	// Import the scene.
	lStatus = lImporter->Import(pScene);
	/*if (lStatus == false && lImporter->GetStatus() == FbxStatus::ePasswordError)
	{
		FBXSDK_printf("Please enter password: ");

		lPassword[0] = '\0';

		FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
			scanf("%s", lPassword);
		FBXSDK_CRT_SECURE_NO_WARNING_END

			FbxString lString(lPassword);

		IOS.SetStringProp(IMP_FBX_PASSWORD, lString);
		IOS.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);

		lStatus = lImporter->Import(pScene);

		if (lStatus == false && lImporter->GetStatus() == FbxStatus::ePasswordError)
		{
			FBXSDK_printf("\nPassword is wrong, import aborted.\n");
		}
	}*/

	/*if (!lStatus || (lImporter->GetStatus() != FbxStatus::eSuccess))
	{
		FBXSDK_printf("********************************************************************************\n");
		if (lStatus)
		{
			FBXSDK_printf("WARNING:\n");
			FBXSDK_printf("   The importer was able to read the file but with errors.\n");
			FBXSDK_printf("   Loaded scene may be incomplete.\n\n");
		}
		else
		{
			FBXSDK_printf("Importer failed to load the file!\n\n");
		}

		if (lImporter->GetStatus() != FbxStatus::eSuccess)
			FBXSDK_printf("   Last error message: %s\n", lImporter->GetStatus().GetErrorString());

		FbxArray<FbxString*> history;
		lImporter->GetStatus().GetErrorStringHistory(history);
		if (history.GetCount() > 1)
		{
			FBXSDK_printf("   Error history stack:\n");
			for (int i = 0; i < history.GetCount(); i++)
			{
				FBXSDK_printf("      %s\n", history[i]->Buffer());
			}
		}
		FbxArrayDelete<FbxString*>(history);
		FBXSDK_printf("********************************************************************************\n");
	}*/

	// Destroy the importer.
	lImporter->Destroy();

	return lStatus;
}

bool fbxWriterReader::writeInternFbxFile(const FBXWritingInfo& info, const MeshGeometries& geometrie, float scale)
{
	if (info.outputPath.empty() || std::filesystem::exists(info.outputPath))
		return false;

	int pFileFormat = -1;

	FbxManager* pManager = FbxManager::Create();
	if (!pManager)
		return false;

	pManager->SetIOSettings(FbxIOSettings::Create(pManager, IOSROOT));
	FbxIOSettings& IOS = *(pManager->GetIOSettings());

	//Create an FBX scene. This object holds most objects imported/exported from/to files.
	FbxScene* pScene = FbxScene::Create(pManager, INTERN_FILE_GUID_V1);
	if (!pScene)
		return false;

	pScene->GetGlobalSettings().SetSystemUnit(fbxsdk::FbxSystemUnit(scale * 100));

	FbxNode* parentNode = FbxNode::Create(pScene, info.meshId.str().c_str());
	
	//Mesh export
	if (!geometrie.indices.empty())
	{
		FbxMesh* pMesh = FbxMesh::Create(pScene, info.meshId.str().c_str());

		assert((geometrie.vertices.size() % 3) == 0);
		assert((geometrie.indices.size() % 3) == 0);
		assert((geometrie.edgesIndices.size() % 2) == 0);

		pMesh->InitControlPoints((int)geometrie.vertices.size() / 3);
		FbxVector4* meshControlPoints = pMesh->GetControlPoints();

		FbxGeometryElementNormal* normalLayer = nullptr;
		if (!geometrie.normals.empty() && geometrie.normals.size() == geometrie.vertices.size())
		{
			normalLayer = pMesh->CreateElementNormal();
			normalLayer->SetMappingMode(FbxGeometryElement::eByControlPoint);
			normalLayer->SetReferenceMode(FbxGeometryElement::eDirect);
		}

		for (int j = 0, iVert = 0; iVert < geometrie.vertices.size(); iVert += 3)
		{
			FbxVector4 controlPoint(geometrie.vertices[iVert], geometrie.vertices[iVert + 1], geometrie.vertices[iVert + 2]);
			meshControlPoints[j] = controlPoint;
			if (normalLayer)
				normalLayer->GetDirectArray().Add(FbxVector4(geometrie.normals[iVert], geometrie.normals[iVert + 1], geometrie.normals[iVert + 2]));
			j++;
		}

		for (auto ind = geometrie.indices.begin(); ind != geometrie.indices.end(); ind += 3)
		{
			pMesh->BeginPolygon();
			pMesh->AddPolygon(*ind);
			pMesh->AddPolygon(*(ind + 1));
			pMesh->AddPolygon(*(ind + 2));
			pMesh->EndPolygon();
		}

		pMesh->BeginAddMeshEdgeIndex();

		for (auto edge = geometrie.edgesIndices.begin(); edge != geometrie.edgesIndices.end(); edge += 2)
			pMesh->AddMeshEdgeIndex(*edge, *(edge + 1), false);

		for (const std::vector<uint32_t>& edges : geometrie.polyligneIndices)
			for (auto edge = edges.begin(); edge + 1 != edges.end(); edge++)
				pMesh->AddMeshEdgeIndex(*edge, *(edge + 1), false);

		pMesh->EndAddMeshEdgeIndex();

		FbxNode* meshNode = FbxNode::Create(pScene, Utils::to_utf8(info.name + L"_mesh").c_str());
		meshNode->SetNodeAttribute(pMesh);
		parentNode->AddChild(meshNode);
	}
	else
	{
		FbxLine* pLine = FbxLine::Create(pScene, info.meshId.str().c_str());

		pLine->InitControlPoints((int)geometrie.vertices.size() / 3);
		FbxVector4* controlPoints = pLine->GetControlPoints();

		for (int j = 0, iVert = 0; iVert < geometrie.vertices.size(); iVert += 3)
		{
			FbxVector4 controlPoint(geometrie.vertices[iVert], geometrie.vertices[iVert + 1], geometrie.vertices[iVert + 2]);
			controlPoints[j] = controlPoint;
			j++;
		}

		for (auto edge = geometrie.edgesIndices.begin(); edge != geometrie.edgesIndices.end(); edge += 2)
		{
			pLine->AddPointIndex(*edge);
			int endInd = pLine->GetIndexArraySize();
			pLine->AddPointIndex(*(edge + 1));
			pLine->AddEndPoint(endInd);
		}

		for (const std::vector<uint32_t>& edges : geometrie.polyligneIndices)
		{
			for (auto edge = edges.begin(); edge != edges.end(); edge++)
				pLine->AddPointIndex(*edge);
			int endInd = pLine->GetIndexArraySize() - 1;
			pLine->AddEndPoint(endInd);
		}

		FbxNode* lineNode = FbxNode::Create(pScene, Utils::to_utf8(info.name + L"_line").c_str());
		lineNode->SetNodeAttribute(pLine);
		parentNode->AddChild(lineNode);
	}

	FbxNode* lRootNode = pScene->GetRootNode();
	lRootNode->AddChild(parentNode);

	bool lStatus = true;

	// Create an exporter.
	FbxExporter* lExporter = FbxExporter::Create(pManager, "");

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

	if (lExporter->Initialize(Utils::to_utf8(info.outputPath.wstring()).c_str(), pFileFormat, pManager->GetIOSettings()) == false)
		return false;

	lStatus = lExporter->Export(pScene);

	lExporter->Destroy();
	pScene->Destroy(true);

	return lStatus;
}

