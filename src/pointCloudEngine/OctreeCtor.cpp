#include "pointCloudEngine/OctreeCtor.h"

#include <chrono>
#include <limits>

//--- Constructors ---//

OctreeCtor::OctreeCtor(const tls::PrecisionType& _precisionType, const tls::PointFormat& _ptFormat)
    : OctreeBase(_precisionType, _ptFormat)
    , m_vertexData(nullptr)
    , m_instData(nullptr)
    , m_octreeDepth(0)
    , m_nbDiscardedPoints(0)
    , m_redundantPointCount(0)
    , m_vertexDataSize(0)
    , m_bbox{ std::numeric_limits<float>::infinity(), - std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), - std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), - std::numeric_limits<float>::infinity() }
{
    // Create the root with a center at (0, 0, 0) and the maximum size of a leaf

    m_rootSize = m_maxLeafSize;
    m_rootPosition[0] = -m_rootSize / 2;
    m_rootPosition[1] = -m_rootSize / 2;
    m_rootPosition[2] = -m_rootSize / 2;

    pushNewCell(m_rootSize, m_rootPosition[0], m_rootPosition[1], m_rootPosition[2], true);
    m_octreeDepth = 0;
    m_uRootCell = 0;
}

OctreeCtor::OctreeCtor(OctreeBase const& _base, BoundingBox const& _bbox)
    : OctreeBase(_base)
    , m_vertexData(nullptr)
    , m_instData(nullptr)
    , m_octreeDepth(0)
    , m_nbDiscardedPoints(0)
    , m_redundantPointCount(0)
    , m_vertexDataSize(0)
    , m_bbox(_bbox)
{
    m_vTempData.resize(m_cellCount);
}

//--- Destructor ---//

OctreeCtor::~OctreeCtor()
{
    delete[] m_vertexData;
    delete[] m_instData;
    for (unsigned i = 0; i < m_vTempData.size(); i++) {
        // m_inPoints.clear();
        delete[] m_vTempData[i].m_outPoints;
        if (m_vTempData[i].m_qspt)
            delete m_vTempData[i].m_qspt;
    }
}

//--- Functions ---//

void OctreeCtor::insertPoint(PointXYZIRGB const& _point)
{
    if (_point.x == 0.f && _point.y == 0.f && _point.z == 0.f)
    {
        m_nbDiscardedPoints++;
        return;
    }

    // Test the bounding box
    {
        if (_point.x < m_bbox.xMin) m_bbox.xMin = _point.x;
        if (_point.x > m_bbox.xMax) m_bbox.xMax = _point.x;
        if (_point.y < m_bbox.yMin) m_bbox.yMin = _point.y;
        if (_point.y > m_bbox.yMax) m_bbox.yMax = _point.y;
        if (_point.z < m_bbox.zMin) m_bbox.zMin = _point.z;
        if (_point.z > m_bbox.zMax) m_bbox.zMax = _point.z;
    }

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
        // In order to keep the root centered to (0, 0, 0) we must split its child
        //   to each child of the new root.
        if (m_vTreeCells[m_uRootCell].m_isLeaf)
        {
            splitCell(m_uRootCell);
        }

        // Insert new Nodes between the root and its childs
        for (uint32_t j = 0; j < 8; ++j)
        {
            if (m_vTreeCells[m_uRootCell].m_children[j] != NO_CHILD)
            {
                // The position of a cell is its inferior corner, so for each new child
                // the position is in [[-size, 0]]^3
                pushNewCell(m_rootSize,
                    -m_rootSize + (j >> 2) * m_rootSize,
                    -m_rootSize + ((j & 2u) >> 1) * m_rootSize,
                    -m_rootSize + (j & 1u) * m_rootSize, false);

                m_vTreeCells[m_vTreeCells.size() - 1].m_children[7 - j] = m_vTreeCells[m_uRootCell].m_children[j];
                m_vTreeCells[m_uRootCell].m_children[j] = (uint32_t)m_vTreeCells.size() - 1;
            }
        }

        // Scale up the root dimensions
        m_rootSize = m_rootSize * 2;
        m_rootPosition[0] = -m_rootSize / 2;
        m_rootPosition[1] = -m_rootSize / 2;
        m_rootPosition[2] = -m_rootSize / 2;

        m_vTreeCells[m_uRootCell].m_size = m_rootSize;
        m_vTreeCells[m_uRootCell].m_position[0] = -m_rootSize / 2;
        m_vTreeCells[m_uRootCell].m_position[1] = -m_rootSize / 2;
        m_vTreeCells[m_uRootCell].m_position[2] = -m_rootSize / 2;

        insertPoint(_point);
    }
}

void OctreeCtor::insertPoint_overwriteI(PointXYZIRGB const& _point)
{
    PointXYZIRGB newPt = _point;
    newPt.i = (_point.r + _point.g + _point.b) / 3;
    insertPoint(newPt);
}

void OctreeCtor::insertPoint_overwriteRGB(PointXYZIRGB const& _point)
{
    PointXYZIRGB newPt = _point;
    newPt.r = _point.i;
    newPt.g = _point.i;
    newPt.b = _point.i;
    insertPoint(newPt);
}

void OctreeCtor::pushNewCell(float _size, float _x, float _y, float _z, bool _isLeaf)
{
    TreeCell cell{ 0, _size, _x, _y, _z, _isLeaf, 0, 0, 0, 0,
                   {NO_CHILD, NO_CHILD, NO_CHILD, NO_CHILD, NO_CHILD, NO_CHILD, NO_CHILD, NO_CHILD },
                   { 0 } };
    TempData data{ 0, 1.f, {0.f, 0.f, 0.f}, {}, nullptr, nullptr };

    //data.m_inPoints = _isLeaf ? new PointXYZIRGB[MAX_POINTS_PER_NODE] : nullptr;

    m_vTreeCells.push_back(cell);
    m_vTempData.push_back(data);
}


void OctreeCtor::insertPointInCell(PointXYZIRGB const& _point, uint32_t _cellId)
{
    if (m_vTreeCells[_cellId].m_isLeaf) {
        if (m_vTempData[_cellId].m_nbPoints < MAX_POINTS_PER_NODE) {
            // Add the point to the arrays
            //m_vTempData[_cellId].m_inPoints[m_vTempData[_cellId].m_nbPoints] = _point;
            m_vTempData[_cellId].m_inPoints.push_back(_point);
            m_vTempData[_cellId].m_nbPoints++;
            return;
        }
        else {
            if (splitCell(_cellId))
                insertPointInCell(_point, _cellId);
            else
                m_nbDiscardedPoints++;
            return;
        }
    }
    else {  // this is an Inner Node

        // Note about performances.
        // 3 'if' imbricated is faster by about 25% than the code below to compute the child index 'j'
        // *****
        // const uint32_t xi = (_point.x < cellPos[0] + halfSize) ? 0u : 1u;
        // const uint32_t yi = (_point.y < cellPos[1] + halfSize) ? 0u : 1u;
        // const uint32_t zi = (_point.z < cellPos[2] + halfSize) ? 0u : 1u;
        // const uint32_t j = (xi << 2) | (yi << 1) | zi;
        // if (m_vTreeCells[_cellId].m_children[j] == NO_CHILD)
        // {
        //     pushNewCell(halfSize,
        //         cellPos[0] + xi * halfSize,
        //         cellPos[1] + yi * halfSize,
        //         cellPos[2] + zi * halfSize, true);
        //     m_vTreeCells[_cellId].m_children[j] = (uint32_t)m_vTreeCells.size() - 1;
        // }
        // return insertPointInCell(_point, m_vTreeCells[_cellId].m_children[j]);
        // *****

        if (_point.x < m_vTreeCells[_cellId].m_position[0] + m_vTreeCells[_cellId].m_size / 2.f)
        {
            if (_point.y < m_vTreeCells[_cellId].m_position[1] + m_vTreeCells[_cellId].m_size / 2.f)
            {
                if (_point.z < m_vTreeCells[_cellId].m_position[2] + m_vTreeCells[_cellId].m_size / 2.f)
                {
                    if (m_vTreeCells[_cellId].m_children[0] == NO_CHILD)
                    {
                        createChildForCell(0, _cellId);
                    }
                    insertPointInCell(_point, m_vTreeCells[_cellId].m_children[0]);
                }
                else
                {
                    if (m_vTreeCells[_cellId].m_children[1] == NO_CHILD)
                    {
                        createChildForCell(1, _cellId);
                    }
                    insertPointInCell(_point, m_vTreeCells[_cellId].m_children[1]);
                }
            }
            else
            {
                if (_point.z < m_vTreeCells[_cellId].m_position[2] + m_vTreeCells[_cellId].m_size / 2.f)
                {
                    if (m_vTreeCells[_cellId].m_children[2] == NO_CHILD)
                    {
                        createChildForCell(2, _cellId);
                    }
                    insertPointInCell(_point, m_vTreeCells[_cellId].m_children[2]);
                }
                else
                {
                    if (m_vTreeCells[_cellId].m_children[3] == NO_CHILD)
                    {
                        createChildForCell(3, _cellId);
                    }
                    insertPointInCell(_point, m_vTreeCells[_cellId].m_children[3]);
                }
            }
        }
        else
        {
            if (_point.y < m_vTreeCells[_cellId].m_position[1] + m_vTreeCells[_cellId].m_size / 2.f)
            {
                if (_point.z < m_vTreeCells[_cellId].m_position[2] + m_vTreeCells[_cellId].m_size / 2.f)
                {
                    if (m_vTreeCells[_cellId].m_children[4] == NO_CHILD)
                    {
                        createChildForCell(4, _cellId);
                    }
                    insertPointInCell(_point, m_vTreeCells[_cellId].m_children[4]);
                }
                else
                {
                    if (m_vTreeCells[_cellId].m_children[5] == NO_CHILD)
                    {
                        createChildForCell(5, _cellId);
                    }
                    insertPointInCell(_point, m_vTreeCells[_cellId].m_children[5]);
                }
            }
            else
            {
                if (_point.z < m_vTreeCells[_cellId].m_position[2] + m_vTreeCells[_cellId].m_size / 2.f)
                {
                    if (m_vTreeCells[_cellId].m_children[6] == NO_CHILD)
                    {
                        createChildForCell(6, _cellId);
                    }
                    insertPointInCell(_point, m_vTreeCells[_cellId].m_children[6]);
                }
                else
                {
                    if (m_vTreeCells[_cellId].m_children[7] == NO_CHILD)
                    {
                        createChildForCell(7, _cellId);
                    }
                    insertPointInCell(_point, m_vTreeCells[_cellId].m_children[7]);
                }
            }
        }
    }
}

void OctreeCtor::createChildForCell(uint32_t _childPos, uint32_t _cellId)
{
    float halfSize = m_vTreeCells[_cellId].m_size / 2.f;
    float* pos = m_vTreeCells[_cellId].m_position;
    bool isLeaf = halfSize <= m_maxLeafSize;

    pushNewCell(halfSize,
        pos[0] + (_childPos >> 2) * halfSize,
        pos[1] + ((_childPos & 2u) >> 1) * halfSize,
        pos[2] + (_childPos & 1u) * halfSize, isLeaf);

    m_vTreeCells[_cellId].m_children[_childPos] = (uint32_t)m_vTreeCells.size() - 1;
}

bool OctreeCtor::splitCell(uint32_t _cellId)
{
    //   We must ensure that the future position of the children cells are compatible
    // with the parent position and size.
    //   The float as a 23bit mantissa, so if we add a two numbers with a relative difference
    // superior to 2^23, the result will be equal to the greater number.
    //   Here, we face the risk that the position is to large compared to the size.
    constexpr float maxFactor = 6291456.f;
    TreeCell& cell = m_vTreeCells[_cellId];
    if (cell.m_position[0] / cell.m_size > maxFactor ||
        cell.m_position[1] / cell.m_size > maxFactor ||
        cell.m_position[2] / cell.m_size > maxFactor ||
        cell.m_size / 2.f < m_minLeafSize)
        return false;

    m_vTreeCells[_cellId].m_isLeaf = false;

    for (const PointXYZIRGB& point : m_vTempData[_cellId].m_inPoints)
    {
        insertPointInCell(point, _cellId);
    }

    m_vTempData[_cellId].m_inPoints.clear();
    m_vTempData[_cellId].m_inPoints.shrink_to_fit();
    m_vTempData[_cellId].m_nbPoints = 0;
    return true;
}


void OctreeCtor::printStats(std::ostream& _os)
{
    if (m_vTreeCells.size() == 0) {
        _os << "Error: no root in octree." << std::endl;
    }

    // Compute stats
    uint32_t maxPtLeaf = 0;
    uint32_t minPtLeaf = MAX_POINTS_PER_NODE;
    uint32_t maxPtNode = 0;
    uint32_t minPtNode = MAX_POINTS_PER_NODE;
    uint32_t maxSptDepth = 0;

    uint64_t leafPointCount = 0;
    uint64_t nodePointCount = 0;
    uint32_t leafCount = 0;
    uint32_t nodeCount = 0;
    uint32_t nodeDepth[9] = { 0 };

    for (unsigned i = 0; i < m_vTreeCells.size(); i++) {
        TreeCell& cell = m_vTreeCells[i];

        if (cell.m_isLeaf) {
            maxPtLeaf = std::max(maxPtLeaf, cell.m_layerIndexes[cell.m_depthSPT]);
            minPtLeaf = std::min(minPtLeaf, cell.m_layerIndexes[cell.m_depthSPT]);
            maxSptDepth = std::max(maxSptDepth, cell.m_depthSPT);

            leafPointCount += cell.m_layerIndexes[cell.m_depthSPT];
            leafCount++;
        }
        else {
            maxPtNode = std::max(maxPtNode, cell.m_layerIndexes[cell.m_depthSPT]);
            minPtNode = std::min(minPtNode, cell.m_layerIndexes[cell.m_depthSPT]);

            nodePointCount += cell.m_layerIndexes[cell.m_depthSPT];
            nodeCount++;
            nodeDepth[cell.m_depthSPT]++;
        }
    }

    uint32_t meanPtLeaf = (uint32_t)(leafPointCount / leafCount);
    uint32_t meanPtNode = 0;
    if (nodeCount != 0)
        meanPtNode = (uint32_t)(nodePointCount / nodeCount);

    // Print stats for leaves
    _os << "---- Octree Statistics ----"
        << "\nLeaf point count    " << leafPointCount
        << "\nPoints in nodes     " << nodePointCount
        << "\nRedundant points    " << m_redundantPointCount
        << "\nDiscarded points    " << m_nbDiscardedPoints
        << "\nOctree depth        " << m_octreeDepth
        << "\nOctree max Size     " << m_rootSize
        << "\nSPT max depth       " << maxSptDepth
        << "\nNode depth repartition: ";
    for (int i = 0; i < 9; ++i)
    {
        _os << " [" << i << "]" << nodeDepth[i];
    }
    _os << "\n---------------------------" << std::endl;

    _os << "\nStats for leaves (" << leafCount << ")"
        << "\nMax point           " << maxPtLeaf
        << "\nMean point          " << meanPtLeaf
        << "\nMin point           " << minPtLeaf
        << "\nStats for nodes (" << nodeCount << ")"
        << "\nMax point           " << maxPtNode
        << "\nMean point          " << meanPtNode
        << "\nMin point           " << minPtNode
        << "\n---------------------------" << std::endl;
}

uint32_t OctreeCtor::getOctreeDepth(uint32_t _cellId)
{
    uint32_t localDepth = 0;

    if (m_vTreeCells[_cellId].m_isLeaf)
        return 0;

    for (uint32_t i = 0; i < 8; i++) {
        if (m_vTreeCells[_cellId].m_children[i] != NO_CHILD)
            localDepth = std::max(localDepth, getOctreeDepth(m_vTreeCells[_cellId].m_children[i]));
    }

    return localDepth + 1;
}

void OctreeCtor::encode(tls::PointFormat ptFormat, std::ostream& _osResult)
{
    setPointFormat(ptFormat);

    m_octreeDepth = getOctreeDepth(m_uRootCell);

    if (m_pointCount == 0)
    {
        std::cout << "ERROR: Octree encoding: try to encode an octree with 0 points" << std::endl;
        return;
    }

    // Reset the point count
    m_pointCount = 0;

    try {
        // Init timers
        std::chrono::system_clock::time_point startTime, nodeTime, endTime;
        startTime = std::chrono::system_clock::now();

        // ****
        createLayerForCell(m_uRootCell);
        // ****

        m_cellCount = (uint32_t)m_vTreeCells.size();

        endTime = std::chrono::system_clock::now();
        float chronoSPT = std::chrono::duration<float, std::ratio<1>>(endTime - startTime).count();
        std::cout << std::setprecision(4) << "SPT Construct time : " << chronoSPT << "s" << std::endl;

        // regroup the informations about the points, leaves, nodes
        regroupData();

        printStats(_osResult);
    }
    catch (std::bad_alloc&) {
        _osResult << "Error: Failed to allocate enough memory during the Octree encoding." << std::endl;
    }
}



void OctreeCtor::constructQSPTForLeaf(uint32_t _cellId)
{
    TreeCell& leaf = m_vTreeCells[_cellId];
    TempData& data = m_vTempData[_cellId];

#ifdef _DEBUG_
    // Check that the cell provided is a correct leaf
    if (!leaf.m_isLeaf || (leaf.m_size / m_precisionValue) > 65536) {
        std::cout << "ERROR: Construct QSPT: leaf not fit for quantization" << std::endl;
        exit(1);
    }
#endif

    // 1. Quantized the points and insert them in a QSPT object
    data.m_instPrecision = m_precisionValue;

    uint32_t uiSize = (uint32_t)(leaf.m_size / m_precisionValue); // garanted to be <= 65536
    data.m_qspt = new QSPT16(data.m_nbPoints, uiSize, m_ptFormat);

    uint32_t discPts = 0;
    // first point already inserted --> start at 1
    for (uint32_t n = 0; n < data.m_nbPoints; n++) {
        Coord16 coordN(data.m_inPoints[n], m_precisionValue, leaf.m_position);
        uint8_t iN = data.m_inPoints[n].i;
        Color24 colorN(data.m_inPoints[n]);

        discPts += data.m_qspt->insertPoint(coordN, iN, colorN);
    }
    data.m_nbPoints -= discPts;
    // Total leaf points in the octree
    m_pointCount += data.m_nbPoints;

    // free the initial point list
    //delete[] data.m_inPoints;
    //data.m_inPoints = nullptr;
    // clear the input points
    data.m_inPoints.clear();
    data.m_inPoints.shrink_to_fit();

    // 2. Get back the points sorted as layers from the QSPT
    leaf.m_depthSPT = data.m_qspt->constructLayers(&data.m_outPoints, leaf.m_dataSize, leaf.m_iOffset, leaf.m_rgbOffset, leaf.m_layerIndexes);

    // FIXME(robin) - The m_layerIndexes should be of size 17 to get a depth of 16
    if (leaf.m_depthSPT == 16)
        leaf.m_depthSPT = 15;

    m_vertexDataSize += leaf.m_dataSize;

#ifdef _DEBUG_
    if (data.m_nbPoints != leaf.m_layerIndexes[leaf.m_depthSPT]) {
        std::cout << "Warning: incoherent number of points after QSPT creation" << std::endl;
        std::cout << "Point count = " << data.m_nbPoints << std::endl;
        std::cout << "Layer count = " << leaf.m_layerIndexes[leaf.m_depthSPT] << std::endl;
        std::cout << "Discarded points for this cell: " << discPts << std::endl;
    }
#endif

    m_nbDiscardedPoints += discPts;

    return;
}


// TODO - Compute and store the layer indexes when we copy the points.
// TODO - Only sum up the total number of points extracted from the children for a specified depth
void OctreeCtor::determineSptDepthFromChildren(uint32_t _cellId)
{
    TreeCell& cell = m_vTreeCells[_cellId];

    // Sum up the points contained for each layers between the children
    uint32_t maxDepthLeaf = 0;
    uint32_t minDepthNode = MAX_SPT_DEPTH;

    uint32_t childLeafLayers[MAX_SPT_DEPTH] = {};
    uint32_t childNodeLayers[MAX_SPT_DEPTH] = {};

    for (uint32_t j = 0; j < 8; ++j)
    {
        if (cell.m_children[j] == NO_CHILD)
            continue;

        TreeCell& child = m_vTreeCells[cell.m_children[j]];

        if (child.m_isLeaf)
        {
            for (uint32_t d = 0; d < MAX_SPT_DEPTH; ++d)
            {
                childLeafLayers[d] += child.m_layerIndexes[d];
            }
        }
        else
        {
            for (uint32_t d = 0; d < MAX_SPT_DEPTH; ++d)
            {
                childNodeLayers[d] += child.m_layerIndexes[d];
            }
            minDepthNode = std::min(minDepthNode, child.m_depthSPT);
        }
    }

    uint32_t leafPointCount = childLeafLayers[MAX_SPT_DEPTH - 1];
    uint32_t maxPointAuthorized = (uint32_t)std::max(LEAF_DECIMATION * (float)MAX_POINTS_PER_NODE, LEAF_DECIMATION * (float)leafPointCount);

    for (uint32_t d = 0; d < MAX_SPT_DEPTH; ++d)
    {
        if (childLeafLayers[d] <= maxPointAuthorized)
            maxDepthLeaf = std::max(maxDepthLeaf, d);
        else
            break;
    }

    cell.m_depthSPT = std::min(maxDepthLeaf, minDepthNode);
    cell.m_depthSPT = std::min(NODE_MAX_DEPTH, cell.m_depthSPT + 1);

    m_redundantPointCount += childNodeLayers[cell.m_depthSPT - 1];

    cell.m_layerIndexes[0] = 1;
    for (uint32_t d = 1; d < cell.m_depthSPT + 1; ++d)
    {
        cell.m_layerIndexes[d] = childLeafLayers[d - 1] + childNodeLayers[d - 1];
    }
    for (uint32_t d = cell.m_depthSPT + 1; d < MAX_SPT_DEPTH; ++d)
    {
        cell.m_layerIndexes[d] = cell.m_layerIndexes[cell.m_depthSPT];
    }

    for (uint32_t d = 0; d < cell.m_depthSPT - 1; ++d)
    {
        m_redundantPointCount -= cell.m_layerIndexes[d];
    }

    m_vTempData[_cellId].m_nbPoints = cell.m_layerIndexes[cell.m_depthSPT];
}

void OctreeCtor::createLayerForCell(uint32_t _cellId)
{
    if (_cellId == NO_CHILD)
        return;

    if (m_vTreeCells[_cellId].m_isLeaf) {
        constructQSPTForLeaf(_cellId);
        return;
    }
    else
    {
        TreeCell& cell = m_vTreeCells[_cellId];
        TempData& data = m_vTempData[_cellId];

        // 1. Create the layer for the children
        for (uint32_t j = 0; j < 8; j++)
        {
            createLayerForCell(cell.m_children[j]); // adapt offsets
        }

        // 2. Determine the SPT depth of this cell based on the point repartition between the childrens
        determineSptDepthFromChildren(_cellId);

        data.m_qspt = new QSPT16(data.m_nbPoints, (1u << cell.m_depthSPT), m_ptFormat);

        // 3. Create and assemble the layers from the children's layers
        for (uint32_t j = 0; j < 8; j++)
        {
            if (cell.m_children[j] == NO_CHILD)
                continue;

            TreeCell& child = m_vTreeCells[cell.m_children[j]];
            TempData& childData = m_vTempData[cell.m_children[j]];

            bool shiftX = j >> 2;
            bool shiftY = (j & 2u) >> 1;
            bool shiftZ = j & 1u;
            data.m_qspt->mergeSPT(cell.m_depthSPT, *childData.m_qspt, child.m_depthSPT, shiftX, shiftY, shiftZ);

            // We do not need the SPT for the remaining treatment
            delete childData.m_qspt;
            childData.m_qspt = nullptr;
        }


        // 3. Set instance precision based on the size and the "SPT depth"
        //    Set the instance position that will help to decode the layer
        float p = cell.m_size / (1u << cell.m_depthSPT); // full precision
        m_vTempData[_cellId].m_instPrecision = p;
        // we translate the origin by p/2 because the each point represent the center of a "pixel"
        m_vTempData[_cellId].m_instPosition[0] = cell.m_position[0] + p / 2;
        m_vTempData[_cellId].m_instPosition[1] = cell.m_position[1] + p / 2;
        m_vTempData[_cellId].m_instPosition[2] = cell.m_position[2] + p / 2;

        // 4. Save the layers from the merge SPT
        uint32_t depth = data.m_qspt->constructLayers(&data.m_outPoints, cell.m_dataSize, cell.m_iOffset, cell.m_rgbOffset, cell.m_layerIndexes);

        m_vertexDataSize += cell.m_dataSize;

        return;
    }
}

void getAlignedSizeAndOffset(uint32_t _nbPoints, tls::PointFormat _format, uint32_t& _iOffset, uint32_t& _rgbOffset, uint32_t& _dataSize)
{
    uint32_t iSize = 0;
    uint32_t rgbSize = 0;
    if (_format == tls::TL_POINT_XYZ_I || _format == tls::TL_POINT_XYZ_I_RGB)
        iSize = _nbPoints * sizeof(uint8_t);

    if (_format == tls::TL_POINT_XYZ_I_RGB || _format == tls::TL_POINT_XYZ_RGB)
        rgbSize = _nbPoints * sizeof(Color24);

    _iOffset = aligned(_nbPoints * sizeof(Coord16), 1);
    _rgbOffset = aligned(_iOffset + iSize, 1);
    _dataSize = aligned(_rgbOffset + rgbSize, 2);
}

void OctreeCtor::regroupData()
{
    // Allocate a large chunck of memory to regroup all the points
    m_vertexData = new char[m_vertexDataSize];

    // Regroup the SPT in the depth order of the main octree
    uint64_t dataStoredOffset = 0;
    uint32_t writeDepth = 0;
    for (uint32_t writeDepth = 0; writeDepth <= m_octreeDepth; writeDepth++)
    {
        //storeChildQSPT(m_uRootCell, 0, writeDepth, dataStoredOffset);
        storeNodeQSPT(m_uRootCell, 0, writeDepth, dataStoredOffset);
    }
    storeLeaves(m_uRootCell, dataStoredOffset);

    // Store the instance data sequentially
    m_instData = new char[m_cellCount * 4 * sizeof(float)];

    for (uint32_t i = 0; i < m_vTreeCells.size(); i++)
    {
        TreeCell& cell = m_vTreeCells[i];
        TempData& data = m_vTempData[i];

        memcpy(m_instData + i * 4 * sizeof(float), cell.m_position, 3 * sizeof(float));
        memcpy(m_instData + i * 4 * sizeof(float) + 3 * sizeof(float), &data.m_instPrecision, sizeof(float));
    }
}

void OctreeCtor::storeNodeQSPT(uint32_t _cellId, uint32_t _depth, uint32_t _writeDepth, uint64_t& _dataStoredOffset)
{
    if (_cellId == NO_CHILD)
        return;

    TreeCell const& cell = m_vTreeCells[_cellId];
    if ((_depth == _writeDepth) && (cell.m_isLeaf == false))
    {
        storeQSPT(_cellId, _dataStoredOffset);
    }
    else if (_depth < _writeDepth)
    {
        // Store the children
        for (size_t j = 0; j < 8; j++)
        {
            storeNodeQSPT(cell.m_children[j], _depth + 1, _writeDepth, _dataStoredOffset);
        }
    }
}

void OctreeCtor::storeLeaves(uint32_t _cellId, uint64_t& _dataStoredOffset)
{
    if (_cellId == NO_CHILD)
        return;

    TreeCell const& cell = m_vTreeCells[_cellId];
    if (cell.m_isLeaf)
        storeQSPT(_cellId, _dataStoredOffset);
    else
    {
        for (size_t j = 0; j < 8; j++)
        {
            storeLeaves(cell.m_children[j], _dataStoredOffset);
        }
    }
}

void OctreeCtor::storeChildQSPT(uint32_t _cellId, uint32_t _depth, uint32_t _writeDepth, uint64_t& _dataStoredOffset)
{
    if (_cellId == NO_CHILD)
        return;

    if (_depth == _writeDepth)
    {
        storeQSPT(_cellId, _dataStoredOffset);
    }
    else if (_depth < _writeDepth)
    {
        TreeCell& cell = m_vTreeCells[_cellId];

        // Store the children
        for (size_t j = 0; j < 8; j++)
        {
            storeChildQSPT(cell.m_children[j], _depth + 1, _writeDepth, _dataStoredOffset);
        }
    }
}

void OctreeCtor::storeQSPT(uint32_t _cellId, uint64_t& _dataStoredOffset)
{
    TreeCell& cell = m_vTreeCells[_cellId];
    TempData& data = m_vTempData[_cellId];

    // NOTE - we can align the data before copying it if need be.
    cell.m_dataOffset = _dataStoredOffset;

    memcpy(m_vertexData + cell.m_dataOffset, data.m_outPoints, cell.m_dataSize);

    _dataStoredOffset = cell.m_dataOffset + cell.m_dataSize;

    delete[] data.m_outPoints;
    data.m_outPoints = nullptr;
}


//------------------------------------------------------//
//   Quantized SPT with point coordinates on 16 bits   //
//----------------------------------------------------//


QSptCell::QSptCell(uint32_t _d, Coord16 _coord, uint8_t _i, Color24 _color) :
    depth(_d),
    children{ 0, 0, 0, 0, 0, 0, 0, 0 },
    coord(_coord),
    i(_i),
    color(_color),
    isLeaf(true)
{ }

QSPT16::QSPT16(uint32_t _nbPoints, uint32_t _size, tls::PointFormat _ptFormat) :
    m_pointFormat(_ptFormat),
    m_size(_size),
    m_nbPoints(0),
    m_maxDepth(0)
{
#ifdef _DEBUG_
    if (m_size == 0 || m_size > 65536) {
        std::cout << "Error: Wrong SPT size at initialization: " << m_size << std::endl;
        exit(1);
    }
#endif
    //memset(m_root.children, 0, 8 * sizeof(uint32_t));
    memset(m_layerSizes, 0, (MAX_SPT_DEPTH + 1) * sizeof(uint32_t));
    // Allocate the space for the estimate number of cells
    m_vCells.reserve(_nbPoints * 2);
}

bool QSPT16::mergeSPT(uint32_t dstDepth, QSPT16& srcSPT, uint32_t srcDepth, bool shiftX , bool shiftY, bool shiftZ)
{
    uint32_t dX;
    uint32_t dY;
    uint32_t dZ;

    // TODO
    dX = shiftX ? (1u << (dstDepth - 1)) : 0u;
    dY = shiftY ? (1u << (dstDepth - 1)) : 0u;
    dZ = shiftZ ? (1u << (dstDepth - 1)) : 0u;

    srcSPT.scaleDown(dstDepth - 1);

    for (uint32_t c = 0; c < srcSPT.m_vCells.size(); ++c)
    {
        QSptCell& srcCell = srcSPT.m_vCells[c];
        if (srcCell.isLeaf && srcCell.depth < dstDepth)
        {
            Coord16 newCoord(srcCell.coord.x + dX, srcCell.coord.y + dY, srcCell.coord.z + dZ);
            insertPoint(newCoord, srcCell.i, srcCell.color);
        }
    }

    return true;
}

bool QSPT16::scaleDown(uint32_t _finalDepth)
{
    uint32_t encodingDepth = 0;
    uint32_t size = m_size;
    while (size > 1)
    {
        size = size >> 1;
        ++encodingDepth;
    }

#ifdef _DEBUG_
    if (m_size > (1u << encodingDepth))
    {
        std::cout << "Error: wrong estimation of SPT depth." << std::endl;
        return false;
    }
#endif

    if (encodingDepth < _finalDepth)
    {
        std::cout << "Warning: the cell cannot be scale down." << std::endl;
        return true;
    }

    uint32_t scaleShift = encodingDepth - _finalDepth;

    // Set the new size
    m_size = 1u << _finalDepth;

    // Scale down reccursively
    bool result = scaleDownCell(0, _finalDepth, scaleShift);
    if (result == false)
        std::cout << "Warning: an unhandle error occured during the SPT scaling down." << std::endl;

#ifdef _DEBUG_
    for (QSptCell& cell : m_vCells)
    {
        // Check that the coordinates are correctly truncated
        if (cell.depth <= _finalDepth && cell.isLeaf)
        {
            if (cell.coord.x >= m_size || cell.coord.y >= m_size || cell.coord.z >= m_size)
                result &= false;
        }
    }
#endif

    return result;
}

bool QSPT16::scaleDownCell(uint32_t _id, uint32_t finalDepth, uint32_t scaleShift)
{
    bool result = true;
    QSptCell& cell = m_vCells[_id];

    if (!cell.isLeaf)
    {
        // Do the reccursion
        for (int j = 0; j < 8; ++j)
        {
            if (cell.children[j] != 0)
                result &= scaleDownCell(cell.children[j], finalDepth, scaleShift);
        }

        // Compute the mean coordinates
        if (cell.depth >= finalDepth)
        {
            uint32_t sumI = 0;
            uint32_t sumRGB[3] = { 0, 0, 0 };
            uint32_t childCount = 0;
            for (int j = 0; j < 8; ++j)
            {
                if (cell.children[j] != 0)
                {
                    ++childCount;
                    QSptCell& child = m_vCells[cell.children[j]];
                    sumI += child.i;
                    sumRGB[0] += child.color.r;
                    sumRGB[1] += child.color.g;
                    sumRGB[2] += child.color.b;
                }
            }

#ifdef _DEBUG_
            if (childCount == 0 || childCount > 8)
            {
                std::cout << "Error 0-7: wrong child count." << std::endl;
                return false;
            }
#endif
            cell.i = (uint8_t)(sumI / childCount);
            cell.color = { (uint8_t)(sumRGB[0] / childCount), (uint8_t)(sumRGB[1] / childCount), (uint8_t)(sumRGB[2] / childCount) };
            
            // Truncate the tree by making the cell a leaf
            memset(cell.children, 0, 8 * sizeof(uint32_t));
            cell.isLeaf = true;
        }
    }

    // shift the coordinates for the cell within the new encodingDepth
    if (cell.depth <= finalDepth)
    {
        cell.coord.x = cell.coord.x >> scaleShift;
        cell.coord.y = cell.coord.y >> scaleShift;
        cell.coord.z = cell.coord.z >> scaleShift;
    }

    return result;
}

// Wraping functions
uint32_t QSPT16::insertPoint(Coord16 _ptCoord, uint8_t _i, Color24 _ptColor)
{
    m_nbPoints++;

    if (m_nbPoints == 1)
    {
        m_vCells.push_back(QSptCell(0, _ptCoord, _i, _ptColor));
        m_layerSizes[0] = 1;
        return 0;
    }
    else
    {
        return insertPoint(0, m_size, 0, 0, 0, _ptCoord, _i, _ptColor);
    }
}

uint32_t QSPT16::insertPoint(uint32_t _id, uint32_t _size, uint16_t _xPos, uint16_t _yPos, uint16_t _zPos, Coord16 _ptCoord, uint8_t _i, Color24 _ptColor)
{
    if (m_vCells[_id].isLeaf) {
        // split the node and insert the point in the same time
        return splitCell(_id, _size, _xPos, _yPos, _zPos, _ptCoord, _i, _ptColor);
    }
    else {
        bool xi = _ptCoord.x >= _xPos + _size / 2;
        bool yi = _ptCoord.y >= _yPos + _size / 2;
        bool zi = _ptCoord.z >= _zPos + _size / 2;
        uint32_t& childIndex = m_vCells[_id].children[xi * 4 + yi * 2 + zi];

        // We verify if the children already exist at this index
        // If there is no children, then we create it and store the point attributes
        if (childIndex == 0)
        {
            childIndex = (uint32_t)m_vCells.size();
            m_vCells.push_back(QSptCell(m_vCells[_id].depth + 1, _ptCoord, _i, _ptColor));
            m_layerSizes[m_vCells[_id].depth + 1]++;
        }
        else
        {
            return insertPoint(childIndex, _size / 2,
                _xPos + xi * _size / 2,
                _yPos + yi * _size / 2,
                _zPos + zi * _size / 2,
                _ptCoord, _i, _ptColor);
        }

        return 0;
    }
}

uint32_t QSPT16::splitCell(uint32_t _id, uint32_t _size, uint16_t _xPos, uint16_t _yPos, uint16_t _zPos, Coord16 _ptCoord, uint8_t _i, Color24 _ptColor)
{
    if (_size <= 1u) {
        //std::cout << "Points with same coordinates found for the precision used." << std::endl;
        return 1;
    }

    m_vCells[_id].isLeaf = false;
    // Increment the max depth of the SPT if we are sure to go beyond
    if (m_vCells[_id].depth >= m_maxDepth) {
        m_maxDepth = m_vCells[_id].depth + 1;
    }
    // the result indicates the number of points that failed at insertion
    // re-insert the point already stored in the node
    uint32_t result = insertPoint(_id, _size, _xPos, _yPos, _zPos, m_vCells[_id].coord, m_vCells[_id].i, m_vCells[_id].color);
    // insert the new point
    result += insertPoint(_id, _size, _xPos, _yPos, _zPos, _ptCoord, _i, _ptColor);

    m_layerSizes[m_vCells[_id].depth + 1]--;

    return result;
}

// return the maximum depth of the SPT
uint32_t QSPT16::constructLayers(char** _pData, uint32_t& _dataSize, uint32_t& _iOffset, uint32_t& _rgbOffset, uint32_t* _layerIndexes)
{
#ifdef _DEBUG_
    if (m_nbPoints == 0)
    {
        std::cout << "ERROR: Construct node layer: 0 points" << std::endl;
        exit(1);
    }
#endif

    getAlignedSizeAndOffset(m_nbPoints, m_pointFormat, _iOffset, _rgbOffset, _dataSize);

    // Allocate the array with the right size
    *_pData = new char[_dataSize];

    // Prepare the write indexes at the begining of each layer
    m_writeLayerIndexes[0] = 0;
    for (uint32_t d = 1; d < m_maxDepth + 1; d++) {
        m_writeLayerIndexes[d] = m_writeLayerIndexes[d - 1] + m_layerSizes[d - 1];
    }

    // Fetch recursively a point for each SPTCell (starting at cellId = 0)
    switch (m_pointFormat) {
    case tls::TL_POINT_XYZ_I:
    {
        uint8_t* pI = (uint8_t*)(*_pData + _iOffset);
        fetchLayers_XYZ_I(m_vCells[0], (Coord16*)*_pData, pI, 0, 0);
        break;
    }
    case tls::TL_POINT_XYZ_RGB:
    {
        Color24* pRGB = (Color24*)(*_pData + _rgbOffset);
        fetchLayers_XYZ_RGB(m_vCells[0], (Coord16*)*_pData, pRGB, 0, 0);
        break;
    }
    case tls::TL_POINT_XYZ_I_RGB:
    {
        uint8_t* pI = (uint8_t*)(*_pData + _iOffset);
        Color24* pRGB = (Color24*)(*_pData + _rgbOffset);
        fetchLayers_XYZ_I_RGB(m_vCells[0], (Coord16*)*_pData, pI, pRGB, 0, 0);
        break;
    }
    default:
        break;
    }

    // Set the all the remaining layer indexes equal to the maximum one
    for (uint32_t d = m_maxDepth + 1; d < MAX_SPT_DEPTH + 1; d++)
    {
        m_writeLayerIndexes[d] = m_writeLayerIndexes[m_maxDepth];
    }

    // FIXME(robin) - add a layer to the destination
    memcpy(_layerIndexes, m_writeLayerIndexes, 16u * sizeof(uint32_t));

    // FIXME(robin)
    // Concat the 15th and 16th layer because we do not have an array large enough in the TreeCell
    _layerIndexes[15] = m_writeLayerIndexes[MAX_SPT_DEPTH];

    return m_maxDepth;
}

// ***** New function with separate attributes *****
// Use a depth offset (depthLeaf) to copy the point in an other layer when needed.
void QSPT16::fetchLayers_XYZ_I(QSptCell const& _cell, Coord16* _pXYZ, uint8_t* _pI, uint32_t _depth, uint32_t _depthLeaf)
{
    if (_cell.isLeaf) {
        memcpy(_pXYZ + m_writeLayerIndexes[_depthLeaf], &_cell.coord, sizeof(Coord16));
        memcpy(_pI + m_writeLayerIndexes[_depthLeaf], &_cell.i, sizeof(uint8_t));

        m_writeLayerIndexes[_depthLeaf]++;
    }
    else {
        bool firstChild = true;
        for (uint32_t j = 0; j < 8; j++) {
            if (_cell.children[j] != 0) {
                // Get the point of the first child
                if (firstChild) {
                    fetchLayers_XYZ_I(m_vCells[_cell.children[j]], _pXYZ, _pI, _depth + 1, _depthLeaf);
                    firstChild = false;
                }
                else fetchLayers_XYZ_I(m_vCells[_cell.children[j]], _pXYZ, _pI, _depth + 1, _depth + 1);
            }
        }
    }
}

void QSPT16::fetchLayers_XYZ_RGB(QSptCell const& _cell, Coord16* _pXYZ, Color24* _pRGB, uint32_t _depth, uint32_t _depthLeaf)
{
    if (_cell.isLeaf) {
        memcpy(_pXYZ + m_writeLayerIndexes[_depthLeaf], &_cell.coord, sizeof(Coord16));
        memcpy(_pRGB + m_writeLayerIndexes[_depthLeaf], &_cell.color, sizeof(Color24));

        m_writeLayerIndexes[_depthLeaf]++;
    }
    else {
        bool firstChild = true;
        for (uint32_t j = 0; j < 8; j++) {
            if (_cell.children[j] != 0) {
                // Get the point of the first child
                if (firstChild) {
                    fetchLayers_XYZ_RGB(m_vCells[_cell.children[j]], _pXYZ, _pRGB, _depth + 1, _depthLeaf);
                    firstChild = false;
                }
                else fetchLayers_XYZ_RGB(m_vCells[_cell.children[j]], _pXYZ, _pRGB, _depth + 1, _depth + 1);
            }
        }
    }
}

void QSPT16::fetchLayers_XYZ_I_RGB(QSptCell const& _cell, Coord16* _pXYZ, uint8_t* _pI, Color24* _pRGB, uint32_t _depth, uint32_t _depthLeaf)
{
    if (_cell.isLeaf) {
        memcpy(_pXYZ + m_writeLayerIndexes[_depthLeaf], &_cell.coord, sizeof(Coord16));
        memcpy(_pI + m_writeLayerIndexes[_depthLeaf], &_cell.i, sizeof(uint8_t));
        memcpy(_pRGB + m_writeLayerIndexes[_depthLeaf], &_cell.color, sizeof(Color24));

        m_writeLayerIndexes[_depthLeaf]++;
    }
    else {
        bool firstChild = true;
        for (uint32_t j = 0; j < 8; j++) {
            if (_cell.children[j] != 0) {
                // Get the point of the first child
                if (firstChild) {
                    fetchLayers_XYZ_I_RGB(m_vCells[_cell.children[j]], _pXYZ, _pI, _pRGB, _depth + 1, _depthLeaf);
                    firstChild = false;
                }
                else fetchLayers_XYZ_I_RGB(m_vCells[_cell.children[j]], _pXYZ, _pI, _pRGB, _depth + 1, _depth + 1);
            }
        }
    }
}

const BoundingBox& OctreeCtor::getBoundingBox() const
{
	return m_bbox;
}
