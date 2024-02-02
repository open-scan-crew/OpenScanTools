#include "models/3d/MeshBuffer.h"

//***************************************
//              Mesh Buffer            **
//***************************************

const VkBuffer& MeshBuffer::getVkBuffer() const
{
    return m_smpBuffer.buffer;
}

const SimpleBuffer& MeshBuffer::getSimpleBuffer() const
{
    return m_smpBuffer;
}

const VkDeviceSize& MeshBuffer::getVertexOffset() const
{
    return m_vertexBufferOffset;
}

const VkDeviceSize& MeshBuffer::getNormalOffset() const
{
    return m_normalBufferOffset;
}

const VkDeviceSize& MeshBuffer::getIndexOffset() const
{
    return m_indexBufferOffset;
}

const std::unordered_map<VkPrimitiveTopology, std::vector<StandardDraw>>& MeshBuffer::getDrawList() const
{
    return m_drawLists;
}

const std::unordered_map<VkPrimitiveTopology, std::vector<IndexedDraw>>& MeshBuffer::getIndexedDrawList() const
{
    return m_indexedDrawLists;
}

void MeshBuffer::addDraw(VkPrimitiveTopology topology, const StandardDraw& draw)
{
    if (m_drawLists.find(topology) != m_drawLists.end())
    {
        m_drawLists.at(topology).emplace_back(draw);
    }
    else
    {
        m_drawLists.insert({ topology, std::vector<StandardDraw>{draw} });
    }
}

void MeshBuffer::addDraws(VkPrimitiveTopology topology, const std::vector<StandardDraw>& draws)
{
    if (m_drawLists.find(topology) != m_drawLists.end())
    {
        std::vector<StandardDraw>& drawList = m_drawLists.at(topology);
        drawList.insert(drawList.end(), draws.begin(), draws.end());
    }
    else
    {
        m_drawLists.insert({ topology, draws });
    }
}

void MeshBuffer::addIndexedDraw(VkPrimitiveTopology topology, const IndexedDraw& draw)
{
    if (m_indexedDrawLists.find(topology) != m_indexedDrawLists.end())
    {
        m_indexedDrawLists.at(topology).emplace_back(draw);
    }
    else
    {
        m_indexedDrawLists.insert({ topology, std::vector<IndexedDraw>{draw} });
    }
}

void MeshBuffer::addIndexedDraws(VkPrimitiveTopology topology, const std::vector<IndexedDraw>& draws)
{
    if (m_indexedDrawLists.find(topology) != m_indexedDrawLists.end())
    {
        std::vector<IndexedDraw>& drawList = m_indexedDrawLists.at(topology);
        drawList.insert(drawList.end(), draws.begin(), draws.end());
    }
    else
    {
        m_indexedDrawLists.insert({ topology, draws });
    }
}

bool MeshBuffer::hasNormals() const
{
    return (m_normalBufferOffset != m_vertexBufferOffset);
}

VkDeviceSize MeshBuffer::getVertexCount() const
{
    if (m_normalBufferOffset != m_vertexBufferOffset)
        return (m_normalBufferOffset / 12u);
    else if (m_indexBufferOffset != m_vertexBufferOffset)
        return (m_indexBufferOffset / 12u);
    else
        return (m_smpBuffer.size / 12u);
}

void MeshBuffer::cleanDrawList()
{
    m_drawLists.clear();
    m_indexedDrawLists.clear();
}
