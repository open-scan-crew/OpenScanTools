#include "io/OctreeBuffer.h"
#include "utils/math/trigo.h"

#include <iostream>

#define OB_NO_CHILD (uint32_t) 0xFFFF4321

OctreeBuffer::OctreeBuffer(float leafSize, tls::PointFormat format, const tls::Transformation& transfo)
    : IPointCloudWriter(format, transfo)
    , m_leafSize(leafSize)
    , m_uRootCell(0)
    , m_pointCount(0)
{
    // Create the root with a center at (0, 0, 0) and the maximum size of a leaf
    m_rootSize = m_leafSize;
    m_rootPosition[0] = -m_rootSize / 2;
    m_rootPosition[1] = -m_rootSize / 2;
    m_rootPosition[2] = -m_rootSize / 2;

    pushNewCell(m_rootSize, m_rootPosition[0], m_rootPosition[1], m_rootPosition[2], true);
    m_uRootCell = 0;
}

OctreeBuffer::~OctreeBuffer()
{
    // Nothing, all points are stored in vectors.
}

uint32_t OctreeBuffer::getRootId() const
{
    return m_uRootCell;
}

void OctreeBuffer::getCellSphere(uint32_t cellId, float& radius, float center[3]) const
{
    if (cellId > m_vCells.size())
        return;

    const Cell& cell = m_vCells[cellId];

    radius = sqrt(3) * cell.m_size;
    center[0] = cell.m_position[0] + cell.m_size / 2;
    center[1] = cell.m_position[1] + cell.m_size / 2;
    center[2] = cell.m_position[2] + cell.m_size / 2;
}

std::vector<uint32_t> OctreeBuffer::getCellChildren(uint32_t cellId) const
{
    std::vector<uint32_t> childrenId;

    if (cellId > m_vCells.size())
        return childrenId;

    for (uint32_t j = 0; j < 8; ++j)
    {
        if (m_vCells[cellId].m_children[j] != OB_NO_CHILD)
            childrenId.push_back(m_vCells[cellId].m_children[j]);
    }

    return childrenId;
}

const PointXYZIRGB* OctreeBuffer::getCellPoints(uint32_t cellId, uint64_t& pointCount)
{
    if (cellId >= m_vCells.size())
    {
#ifdef _DEBUG_
        std::cout << "Error: OctreeBuffer::getCellPoints() -> Wrong cell ID: " << cellId << std::endl;
#endif
        pointCount = 0;
        return nullptr;
    }

    pointCount = m_vCells[cellId].m_points.size();
    return m_vCells[cellId].m_points.data();
}

bool OctreeBuffer::copyCellPoints(uint32_t cellId, PointXYZIRGB* dstPoints, uint64_t dstSize, uint64_t& dstOffset)
{
    if (cellId >= m_vCells.size())
    {
#ifdef _DEBUG_
        std::cout << "Error: OctreeBuffer::getCellPoints() -> Wrong cell ID: " << cellId << std::endl;
#endif
        return false;
    }

    if (!m_vCells[cellId].m_isLeaf)
    {
#ifdef _DEBUG_
        std::cout << "Error: OctreeBuffer::getCellPoints() -> The cell " << cellId << " is not a leaf." << std::endl;
#endif
        return false;
    }

    const uint64_t& nbOfPoints = m_vCells[cellId].m_points.size();

    // Check that there is enough space in the destination buffer
    if (dstOffset + nbOfPoints > dstSize)
        return false;

    memcpy(dstPoints + dstOffset, m_vCells[cellId].m_points.data(), nbOfPoints * sizeof(PointXYZIRGB));
    dstOffset += nbOfPoints;
    return true;
}

bool OctreeBuffer::isPointCloudWriterOpen() const
{
    return true;
}

bool OctreeBuffer::addPoints_localDst(PointXYZIRGB* srcBuf, uint64_t srcSize, tls::PointFormat srcFormat)
{
    if (srcFormat == m_format)
    {
        for (uint64_t n = 0; n < srcSize; ++n)
        {
            PointXYZIRGB newPoint = convert_keepIRGB(srcBuf[n]);
            insertPoint(newPoint);
        }
    }
    else if (srcFormat == tls::PointFormat::TL_POINT_XYZ_RGB)
    {
        for (uint64_t n = 0; n < srcSize; ++n)
        {
            PointXYZIRGB newPoint = convert_overwriteI(srcBuf[n]);
            insertPoint(newPoint);
        }
    }
    else if (srcFormat == tls::PointFormat::TL_POINT_XYZ_I)
    {
        for (uint64_t n = 0; n < srcSize; ++n)
        {
            PointXYZIRGB newPoint = convert_overwriteRGB(srcBuf[n]);
            insertPoint(newPoint);
        }
    }
    else
    {
        // ERROR
        return false;
    }
    return true;
}

bool OctreeBuffer::addPoints_global(PointXYZIRGB* srcBuf, uint64_t srcSize, tls::PointFormat srcFormat)
{
    if (m_transfo == tls::Transformation{{0, 0, 0, 1}, { 0, 0, 0 }})
        return addPoints_localDst(srcBuf, srcSize, srcFormat);

    glm::mat4 matrix = tls::math::getInverseTransformMatrix(m_transfo.translation, m_transfo.quaternion);

    if (srcFormat == m_format)
    {
        for (uint64_t n = 0; n < srcSize; ++n)
        {
            PointXYZIRGB newPoint = convert_keepIRGB(srcBuf[n], matrix);
            insertPoint(newPoint);
        }
    }
    else if (srcFormat == tls::PointFormat::TL_POINT_XYZ_RGB)
    {
        for (uint64_t n = 0; n < srcSize; ++n)
        {
            PointXYZIRGB newPoint = convert_overwriteI(srcBuf[n], matrix);
            insertPoint(newPoint);
        }
    }
    else if (srcFormat == tls::PointFormat::TL_POINT_XYZ_I)
    {
        for (uint64_t n = 0; n < srcSize; ++n)
        {
            PointXYZIRGB newPoint = convert_overwriteRGB(srcBuf[n], matrix);
            insertPoint(newPoint);
        }
    }
    else
    {
        // ERROR
        return false;
    }
    return true;
}

bool OctreeBuffer::addPoints_localSrc(PointXYZIRGB* srcBuf, uint64_t srcSize, const tls::Transformation& srcTransfo, tls::PointFormat srcFormat)
{
    if (srcTransfo == m_transfo)
        return addPoints_localDst(srcBuf, srcSize, srcFormat);

    glm::mat4 finalMatrix = tls::math::getInverseTransformMatrix(m_transfo.translation, m_transfo.quaternion);
    {
        glm::mat4 srcMatrix = tls::math::getTransformMatrix(srcTransfo.translation, srcTransfo.quaternion);
        finalMatrix *= srcMatrix;
    }

    if (srcFormat == m_format)
    {
        for (uint64_t n = 0; n < srcSize; ++n)
        {
            PointXYZIRGB newPoint = convert_keepIRGB(srcBuf[n], finalMatrix);
            insertPoint(newPoint);
        }
    }
    else if (srcFormat == tls::PointFormat::TL_POINT_XYZ_RGB)
    {
        for (uint64_t n = 0; n < srcSize; ++n)
        {
            PointXYZIRGB newPoint = convert_overwriteI(srcBuf[n], finalMatrix);
            insertPoint(newPoint);
        }
    }
    else if (srcFormat == tls::PointFormat::TL_POINT_XYZ_I)
    {
        for (uint64_t n = 0; n < srcSize; ++n)
        {
            PointXYZIRGB newPoint = convert_overwriteRGB(srcBuf[n], finalMatrix);
            insertPoint(newPoint);
        }
    }
    else
    {
        // ERROR
        return false;
    }
    return true;
}

void OctreeBuffer::insertPoint(const PointXYZIRGB& _point)
{
    // Test if the point is inside root node
    if (_point.x >= m_rootPosition[0] &&
        _point.y >= m_rootPosition[1] &&
        _point.z >= m_rootPosition[2] &&
        _point.x <= m_rootPosition[0] + m_rootSize &&
        _point.y <= m_rootPosition[1] + m_rootSize &&
        _point.z <= m_rootPosition[2] + m_rootSize)
    {
        insertPointInCell(_point, m_uRootCell);
        m_pointCount++;
    }
    else {
        // We must increase the root size
        // -> In order to keep the root centered to (0, 0, 0) we must split its child
        //    to each child of the new root.
        if (m_vCells[m_uRootCell].m_isLeaf)
        {
            splitCell(m_uRootCell);
        }

        // Insert new Nodes between the root and its childs
        for (uint32_t j = 0; j < 8; ++j)
        {
            if (m_vCells[m_uRootCell].m_children[j] != OB_NO_CHILD)
            {
                // The position of a cell is its inferior corner, so for each new child
                // the position is in [[-size, 0]]^3
                pushNewCell(m_rootSize,
                    -m_rootSize + (j >> 2) * m_rootSize,
                    -m_rootSize + ((j & 2u) >> 1) * m_rootSize,
                    -m_rootSize + (j & 1u) * m_rootSize, false);

                m_vCells[m_vCells.size() - 1].m_children[7 - j] = m_vCells[m_uRootCell].m_children[j];
                m_vCells[m_uRootCell].m_children[j] = m_vCells.size() - 1;
            }
        }

        // Scale up the root dimensions
        m_rootSize = m_rootSize * 2;
        m_rootPosition[0] = -m_rootSize / 2;
        m_rootPosition[1] = -m_rootSize / 2;
        m_rootPosition[2] = -m_rootSize / 2;

        m_vCells[m_uRootCell].m_size = m_rootSize;
        m_vCells[m_uRootCell].m_position[0] = -m_rootSize / 2;
        m_vCells[m_uRootCell].m_position[1] = -m_rootSize / 2;
        m_vCells[m_uRootCell].m_position[2] = -m_rootSize / 2;

        insertPoint(_point);
    }
}

void OctreeBuffer::insertPoints(const PointXYZIRGB* points, uint64_t ptCount)
{
    for (uint64_t n = 0; n < ptCount; ++n)
    {
        insertPoint(points[n]);
    }
}

const std::vector<Cell>& OctreeBuffer::getData() const
{
    return m_vCells;
}

int64_t OctreeBuffer::getCellCount() const
{
    return m_vCells.size();
}

uint64_t OctreeBuffer::getPointCount() const
{
    return m_pointCount;
}

void OctreeBuffer::pushNewCell(float _size, float _x, float _y, float _z, bool _isLeaf)
{
    Cell cell{ std::vector<PointXYZIRGB>(), _size, _x, _y, _z, _isLeaf, 
        {OB_NO_CHILD, OB_NO_CHILD, OB_NO_CHILD, OB_NO_CHILD, OB_NO_CHILD, OB_NO_CHILD, OB_NO_CHILD, OB_NO_CHILD } };

    m_vCells.push_back(cell);
}

void OctreeBuffer::insertPointInCell(PointXYZIRGB const& _point, uint32_t _cellId)
{
    if (m_vCells[_cellId].m_isLeaf)
    {
        m_vCells[_cellId].m_points.push_back(_point);
    }
    else
    {
        if (_point.x < m_vCells[_cellId].m_position[0] + m_vCells[_cellId].m_size / 2.f)
        {
            if (_point.y < m_vCells[_cellId].m_position[1] + m_vCells[_cellId].m_size / 2.f)
            {
                if (_point.z < m_vCells[_cellId].m_position[2] + m_vCells[_cellId].m_size / 2.f)
                {
                    if (m_vCells[_cellId].m_children[0] == OB_NO_CHILD)
                    {
                        createChildForCell(0, _cellId);
                    }
                    insertPointInCell(_point, m_vCells[_cellId].m_children[0]);
                }
                else
                {
                    if (m_vCells[_cellId].m_children[1] == OB_NO_CHILD)
                    {
                        createChildForCell(1, _cellId);
                    }
                    insertPointInCell(_point, m_vCells[_cellId].m_children[1]);
                }
            }
            else
            {
                if (_point.z < m_vCells[_cellId].m_position[2] + m_vCells[_cellId].m_size / 2.f)
                {
                    if (m_vCells[_cellId].m_children[2] == OB_NO_CHILD)
                    {
                        createChildForCell(2, _cellId);
                    }
                    insertPointInCell(_point, m_vCells[_cellId].m_children[2]);
                }
                else
                {
                    if (m_vCells[_cellId].m_children[3] == OB_NO_CHILD)
                    {
                        createChildForCell(3, _cellId);
                    }
                    insertPointInCell(_point, m_vCells[_cellId].m_children[3]);
                }
            }
        }
        else
        {
            if (_point.y < m_vCells[_cellId].m_position[1] + m_vCells[_cellId].m_size / 2.f)
            {
                if (_point.z < m_vCells[_cellId].m_position[2] + m_vCells[_cellId].m_size / 2.f)
                {
                    if (m_vCells[_cellId].m_children[4] == OB_NO_CHILD)
                    {
                        createChildForCell(4, _cellId);
                    }
                    insertPointInCell(_point, m_vCells[_cellId].m_children[4]);
                }
                else
                {
                    if (m_vCells[_cellId].m_children[5] == OB_NO_CHILD)
                    {
                        createChildForCell(5, _cellId);
                    }
                    insertPointInCell(_point, m_vCells[_cellId].m_children[5]);
                }
            }
            else
            {
                if (_point.z < m_vCells[_cellId].m_position[2] + m_vCells[_cellId].m_size / 2.f)
                {
                    if (m_vCells[_cellId].m_children[6] == OB_NO_CHILD)
                    {
                        createChildForCell(6, _cellId);
                    }
                    insertPointInCell(_point, m_vCells[_cellId].m_children[6]);
                }
                else
                {
                    if (m_vCells[_cellId].m_children[7] == OB_NO_CHILD)
                    {
                        createChildForCell(7, _cellId);
                    }
                    insertPointInCell(_point, m_vCells[_cellId].m_children[7]);
                }
            }
        }
    }
}

void OctreeBuffer::createChildForCell(uint32_t _childPos, uint32_t _cellId)
{
    float halfSize = m_vCells[_cellId].m_size / 2.f;
    float* pos = m_vCells[_cellId].m_position;
    bool isLeaf = (halfSize <= m_leafSize);

    pushNewCell(halfSize,
        pos[0] + (_childPos >> 2) * halfSize,
        pos[1] + ((_childPos & 2u) >> 1) * halfSize,
        pos[2] + (_childPos & 1u) * halfSize, isLeaf);

    m_vCells[_cellId].m_children[_childPos] = (uint32_t)m_vCells.size() - 1;
}

void OctreeBuffer::splitCell(uint32_t _cellId)
{
    m_vCells[_cellId].m_isLeaf = false;

    // the result indicate the number of points that failed at insertion

    for (const PointXYZIRGB& point : m_vCells[_cellId].m_points) {
        insertPointInCell(point, _cellId);
    }

    m_vCells[_cellId].m_points.clear();
}