#include "io/MeshObjectTypes.h"


void MeshGeometries::clear() {
	vertices.clear();
	normals.clear();
	texcoords.clear();
	indices.clear();
	edgesIndices.clear();
}

void MeshGeometries::clearMaps()
{
	verticesMap.clear();
}

void MeshGeometries::merge(const MeshGeometries& geom)
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

uint32_t MeshGeometries::addVertice(const std::array<float, 3>& point, bool useVerticesMap)
{
	uint32_t pointInd = 0;

	if (useVerticesMap && verticesMap.find(point) != verticesMap.end())
		pointInd = verticesMap.at(point);
	else {
		pointInd = (uint32_t)vertices.size() / 3;
		vertices.push_back(point[0]);
		vertices.push_back(point[1]);
		vertices.push_back(point[2]);
		if (useVerticesMap)
			verticesMap[{point[0], point[1], point[2]}] = pointInd;
	}

	return pointInd;
}

MeshObjInputData::MeshObjInputData(std::filesystem::path path, bool generateEdges, bool isMerge, bool centerPosition, MeshId internLoadMeshId, int lod) {
	this->path = path;
	this->extension = getFileType(path.extension());
	this->lod = lod;

	this->generateEdges = generateEdges;
	this->isMerge = isMerge;

	this->centerPosition = centerPosition;

	this->meshScale = -1.f;
}

bool MeshObjInputData::operator==(const MeshObjInputData& comp) const
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