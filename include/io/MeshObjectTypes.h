#ifndef MESH_OBJECT_TYPES_H
#define MESH_OBJECT_TYPES_H

#include "io/FileUtils.h"
#include "models/3d/MeshId.h"

#include <array>
#include <filesystem>
#include <glm/glm.hpp>

struct HashVec3 {
    std::hash<float> hasher;
    size_t operator() (const std::array<float, 3>& key) const {
        size_t h = 0;
        for (size_t i = 0; i < 3; ++i)
            h = h * 31 + hasher(key.at(i));

        return h;
    }
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

    void clear();
    void clearMaps();
    void merge(const MeshGeometries& geom);
    uint32_t addVertice(const std::array<float, 3>& point, bool useVerticesMap);
};

//std::filesystem::path path, FileType extension, bool generateEdges, bool isMerge, bool centerPosition = true, MeshId internLoadMeshId = xg::Guid(), LODValue lod = 60
struct MeshObjInputData {
    MeshObjInputData(std::filesystem::path path, bool generateEdges, bool isMerge, bool centerPosition = true, MeshId internLoadMeshId = xg::Guid(), int lod = 60);

    std::filesystem::path path;
    FileType extension;

    int lod;
    bool generateEdges;
    bool isMerge;

    float meshScale;
    bool centerPosition;

    bool operator==(const MeshObjInputData& comp) const;
};

#endif