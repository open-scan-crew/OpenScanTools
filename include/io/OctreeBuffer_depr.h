#ifndef OCTREE_BUFFER_H
#define OCTREE_BUFFER_H

//#include "models/pointCloud/TLS.h"
#include "models/pointCloud/Point.h"
#include "io/imports/IOctreeReader.h"
#include "models/pointCloud/IPointCloudWriter.h"

#include <cstdint>
#include <vector>
//#include <fstream>


// Sizeof Cell = 16 + 4 + 12 + 4 + 32 = 76
struct Cell
{
    std::vector<PointXYZIRGB> m_points;
    float m_size;
    float m_position[3];
    bool m_isLeaf;
    uint32_t m_children[8];
};

/*
struct Node
{
    float m_size;
    float m_position[3];
    uint32_t pointCount;
    uint32_t m_children[8];
};

struct Leaf
{
    float m_size;
    float m_position[3];
    //uint32_t pointCount;
    std::vector<PointXYZIRGB> m_points;
};
*/

class OctreeBuffer : public IOctreeReader, public IPointCloudWriter // : public IPointReader
{
public:
    OctreeBuffer(float leafSize, tls::PointFormat format, const tls::Transformation& transfo);
    ~OctreeBuffer();

    // IOctreeReader
    uint32_t getRootId() const override;
    void getCellSphere(uint32_t cellId, float& radius, float center[3]) const override;
    std::vector<uint32_t> getCellChildren(uint32_t cellId) const override;

    const PointXYZIRGB* getCellPoints(uint32_t cellId, uint64_t& pointCount) override;
    bool copyCellPoints(uint32_t cellId, PointXYZIRGB* dstPoints, uint64_t dstSize, uint64_t& dstOffset) override;

    // IPointReader
    //const PointXYZIRGB* getCellPoints(uint32_t cellId, uint64_t& pointCount);
    //bool copyCellPoints(uint32_t cellId, PointXYZIRGB* dstPoints, uint64_t dstSize, uint64_t& dstOffset);

    // IPointCloudWriter
    bool isPointCloudWriterOpen() const override;
    bool addPoints_localDst(PointXYZIRGB* srcBuf, uint64_t srcSize, tls::PointFormat srcFormat) override;
    bool addPoints_global(PointXYZIRGB* srcBuf, uint64_t srcSize, tls::PointFormat srcFormat) override;
    bool addPoints_localSrc(PointXYZIRGB* srcBuf, uint64_t srcSize, const tls::Transformation& srcTransfo, tls::PointFormat srcFormat) override;

    // 
    void insertPoint(const PointXYZIRGB& point);
    void insertPoints(const PointXYZIRGB* points, uint64_t ptCount);

    const std::vector<Cell>& getData() const;
    int64_t getCellCount() const;
    uint64_t getPointCount() const;

protected:
    void pushNewCell(float size, float x, float y, float z, bool isLeaf);
    void insertPointInCell(PointXYZIRGB const& _point, uint32_t _cellId);
    void createChildForCell(uint32_t _childPos, uint32_t _cellId);
    void splitCell(uint32_t _cellId);

protected:
    const float m_leafSize;
    float m_rootSize;
    float m_rootPosition[3];

    uint64_t m_pointCount;

    uint32_t m_uRootCell;
    std::vector<Cell> m_vCells;
};

#endif  // !OCTREE_BUFFER_H