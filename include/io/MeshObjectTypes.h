#ifndef MESH_OBJECT_TYPES_H
#define MESH_OBJECT_TYPES_H

#include "models/3d/ManipulationTypes.h"
#include "io/FileUtils.h"
#include "models/3d/MeshId.h"
#include "io/imports/ImportTypes.h"

#include <filesystem>
#include <glm/glm.hpp>
#include "utils/math/basic_functions.h"

//value between 1 and 100
typedef int LODValue; 

struct FileInputData {
	std::filesystem::path file;
	float scale;
	Selection up;
	Selection forward;
	bool isMerge;
	bool truncateCoordinatesAsTheScans = false;
	PositionOptions posOption;
	glm::vec3 position;
	FileType extension;
	LODValue lod;
};


struct MeshGeometries
{
	std::unordered_map<std::array<float, 3>, uint32_t, HashVec3> verticesMap;
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> texcoords;
	std::vector<uint32_t> indices;
	std::vector<uint32_t> edgesIndices;
	std::vector<std::vector<uint32_t>> polyligneIndices;

	void clear() {
		vertices.clear();
		normals.clear();
		texcoords.clear();
		indices.clear();
		edgesIndices.clear();
	}

	void clearMaps()
	{
		verticesMap.clear();
	}

	void merge(const MeshGeometries& geom)
	{
		uint32_t verticesOldSize = (uint32_t)vertices.size() / (uint32_t)3;
		vertices.insert(vertices.end(), geom.vertices.begin(), geom.vertices.end());
		normals.insert(normals.end(), geom.normals.begin(), geom.normals.end());

		for (const uint32_t& ind : geom.indices)
			indices.push_back(ind + verticesOldSize);

		for (const uint32_t& ind : geom.edgesIndices)
			edgesIndices.push_back(ind + verticesOldSize);

		for (std::vector<uint32_t> inds : geom.polyligneIndices)
		{
			std::for_each(inds.begin(), inds.end(), [verticesOldSize](uint32_t& ind) {ind += verticesOldSize; });
			polyligneIndices.push_back(inds);
		}

		std::unordered_map<std::array<float, 3>, uint32_t, HashVec3> copyVerticesMap(geom.verticesMap);
		for (auto& kv : copyVerticesMap)
			copyVerticesMap[kv.first] = kv.second + verticesOldSize;
		verticesMap.merge(copyVerticesMap);
	}

	uint32_t addVertice(const std::array<float, 3>& point, bool useVerticesMap)
	{
		uint32_t pointInd = 0;

		if (useVerticesMap && verticesMap.find(point) != verticesMap.end())
			pointInd = verticesMap.at(point);
		else {
			pointInd = (uint32_t)vertices.size() / 3;
			vertices.push_back(point[0]);
			vertices.push_back(point[1]);
			vertices.push_back(point[2]);
			if(useVerticesMap)
				verticesMap[{point[0], point[1], point[2]}] = pointInd;
		}

		return pointInd;
	}
};

//std::filesystem::path path, FileType extension, bool generateEdges, bool isMerge, bool centerPosition = true, MeshId internLoadMeshId = xg::Guid(), LODValue lod = 60
struct MeshObjInputData {
	MeshObjInputData(std::filesystem::path path, bool generateEdges, bool isMerge, bool centerPosition = true, MeshId internLoadMeshId = xg::Guid(), LODValue lod = 60) {
		this->path = path;
		this->extension = getFileType(path.extension());
		this->lod = lod;

		this->generateEdges = generateEdges;
		this->isMerge = isMerge;

		this->centerPosition = centerPosition;

		this->meshScale = -1.f;
	}
	std::filesystem::path path;
	FileType extension;

	LODValue lod;
	bool generateEdges;
	bool isMerge;

	float meshScale;
	bool centerPosition;


	bool operator==(const MeshObjInputData& comp) const
	{
		return (
			path.wstring().compare(comp.path.wstring()) == 0
			&& extension == comp.extension
			&& lod == comp.lod
			&& generateEdges == comp.generateEdges
			&& isMerge == comp.isMerge
			&& centerPosition == comp.centerPosition

			);

	}
};

#endif