#ifndef _FBX_READER_H_
#define _FBX_READER_H_

#include "utils/math/basic_functions.h"
#include <filesystem>
#include <unordered_map>
#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>

#include "vulkan/Graph/MemoryReturnCode.h"

#include "io/imports/IMeshReader.h"

struct FBXWritingInfo
{
	std::filesystem::path outputPath;
	std::wstring name;
	xg::Guid meshId;
};

class Controller;

class fbxWriterReader : public IMeshReader
{
public:
	fbxWriterReader(Controller* pController, const MeshObjInputData& inputInfo);
	~fbxWriterReader();

	bool read() override;
	ObjectAllocation::ReturnCode generateGeometries() override;
	xg::Guid getLoadedMeshId() const override;

	ObjectAllocation::ReturnCode generateInternalFilesGeometries();

	static bool writeInternFbxFile(const FBXWritingInfo& info, const MeshGeometries& geom, float scale);

private:
	bool LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename);
	void recGetChildMesh(FbxNode* node);

	std::array<float, 3> transformFbxVertex(FbxNode* node, const FbxVector4& vertice, bool onlyRotation);

	bool loadNode(FbxNode* node, MeshGeometries& geom);
	bool loadMesh(FbxNode* mesh, MeshGeometries& geom);
	bool loadLine(FbxNode* mesh, MeshGeometries& geom);

	
private:
	FbxScene* m_scene;
	FbxManager* m_manager;

	std::vector<FbxNode*> m_models;

	float m_fileScale;

	int m_internLoadVersion;
};

#endif // !_STEP_FILE_READER_H_
