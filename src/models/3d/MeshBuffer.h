#ifndef MESH_BUFFER_H
#define MESH_BUFFER_H

#include "vulkan/vulkan.h"
#include "pointCloudEngine/SmartBuffer.h"

#include <unordered_map>

class IMeshReader;
class MeshManager;

struct StandardDraw
{
    uint32_t firstVertex;
    uint32_t vertexCount;
};

struct IndexedDraw
{
    int32_t vertexOffset;
    uint32_t firstIndex;
    uint32_t indexCount;
};

typedef enum TlTopologyFlagBits {
    TL_TOPOLOGY_LINE = 0x00000001,
    TL_TOPOLOGY_TRIANGLE = 0x00000002,
    TL_TOPOLOGY_ALL = TL_TOPOLOGY_LINE | TL_TOPOLOGY_TRIANGLE,
    TL_TOPOLOGY_NONE = 0
} TlTopologyFlagBits;
typedef uint32_t TlTopologyFlags;

/*
typedef enum TlVertexInputFlagBits {
    TL_VERTEX_INPUT_POSITION = 0x00000001,
    TL_VERTEX_INPUT_NORMAL = 0x00000002,
    TL_VERTEX_INPUT_COLOR = 0x00000004,
    TL_VERTEX_INPUT_UV = 0x00000008,
} TlVertexInputFlagBits;
typedef uint32_t TlVertexInputFlags;
*/

class MeshBuffer
{
public:
    const VkBuffer& getVkBuffer() const;
    const SimpleBuffer& getSimpleBuffer() const;
    const VkDeviceSize& getVertexOffset() const;
    const VkDeviceSize& getNormalOffset() const;
    const VkDeviceSize& getIndexOffset() const;
    const std::unordered_map<VkPrimitiveTopology, std::vector<StandardDraw>>& getDrawList() const;
    const std::unordered_map<VkPrimitiveTopology, std::vector<IndexedDraw>>& getIndexedDrawList() const;

    void addDraw(VkPrimitiveTopology topology, const StandardDraw& draw);
    void addDraws(VkPrimitiveTopology topology, const std::vector<StandardDraw>& draws);
    void addIndexedDraw(VkPrimitiveTopology topology, const IndexedDraw& draw);
    void addIndexedDraws(VkPrimitiveTopology topology, const std::vector<IndexedDraw>& draws);

    bool hasNormals() const;
    VkDeviceSize getVertexCount() const;
    void cleanDrawList();

private:
    friend IMeshReader;
    friend MeshManager;
    SimpleBuffer m_smpBuffer;
    VkDeviceSize m_vertexBufferOffset;
    VkDeviceSize m_normalBufferOffset;
    VkDeviceSize m_indexBufferOffset;
    std::unordered_map<VkPrimitiveTopology, std::vector<StandardDraw>> m_drawLists;
    std::unordered_map<VkPrimitiveTopology, std::vector<IndexedDraw>> m_indexedDrawLists;
};

enum class GenericMeshType {
    Cube,
    Cylinder,
    Sphere,
    //Torus,
    Camera_Perspective,
    Camera_Orthographic
};


struct GenericMeshId
{
    GenericMeshType type;
    union Shape
    {
        uint32_t hash;
        struct Cube
        {
            uint16_t faceCount; // Pas vraiment utile -> que mettre d'autre ?
        } cube;
        struct Cylinder
        {
            uint16_t sectorCount;
        } cylinder;
        struct Sphere
        {
            uint16_t longitudeCount;
            uint16_t latitudeCount;
        } sphere;
    } shape;
};

template<>
struct std::hash<GenericMeshId>
{
    std::size_t operator()(GenericMeshId const& mid) const noexcept
    {
        return (((size_t)mid.type << 32) + (size_t)mid.shape.hash);
    }
};

template<>
struct std::equal_to<GenericMeshId>
{
    bool operator()(const GenericMeshId& lhs, const GenericMeshId& rhs) const
    {
        return (lhs.type == rhs.type) && (lhs.shape.hash == rhs.shape.hash);
    }
};

#endif