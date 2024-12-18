/*** Octree.cpp ***************************************************************
** Author           - Robin Kervadec                                         **
**                                                                           **
** TODO insert description                                                   **
**                                                                           **
****** Copyright (C) 2018 TagLabs ********************************************/

#include "Octree_real.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <algorithm>
#include <chrono>
#include "math.h"

#include "../../glm/0.9.8.5/glm/gtx/norm.hpp"

//--- Default constructors ---//
NestedOctree::NestedOctree() : NestedOctree(TL_OCTREE_UNCOMPRESSED)
{ }

ConstructNode::ConstructNode() : ConstructNode(-1.f, 0.f, 0.f, 0.f, true)
{ }

ExploitNode::ExploitNode() :
    m_children{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
    m_layerIndexes(nullptr)
{ }

//--- Constructors ---//

NestedOctree::NestedOctree(OctreeFormat _format) :
    m_format(_format),
    m_rootSize(0.0),
    m_rootPosition{0.f, 0.f, 0.f},
    m_status(OCTREE_OPEN),
    m_nbPoints(0),
    m_nbInnerPoints(0),
    m_nbDiscardedPoints(0),
    m_nbLeaves(0),
    m_nbInnerNodes(0),
    m_nbVertex(0),
    m_nbInst(0),
    m_vertexData(nullptr),
    m_instData(nullptr),
    m_constructRoot(nullptr),
    m_exploitRoot(nullptr)
{
    switch (m_format) {
    case TL_OCTREE_UNCOMPRESSED:
        m_sizePoint = 16;
        m_rootSize = 1.f;
        break;
    case TL_OCTREE_1MM:
        m_sizePoint = SIZE_QUANT_POINT;
        m_rootSize = 65.530f;
        break;
    case TL_OCTREE_100UM:
        m_sizePoint = SIZE_QUANT_POINT;
        m_rootSize = 6.5530f;
        break;
    case TL_OCTREE_10UM:
        m_sizePoint = SIZE_QUANT_POINT;
        m_rootSize = 0.65530f;
        break;
    default:
        m_sizePoint = 16;
        m_rootSize = 1.f;
        break;
    }

    // The NestedOctree is created empty
    std::cout << "Size of NestedOctree = " << sizeof(NestedOctree) << " bytes" << std::endl;
    std::cout << "Size of ConstructNode = " << sizeof(ConstructNode) << " bytes" << std::endl;
    std::cout << "Size of ExploitNode = " << sizeof(ExploitNode) << " bytes" << std::endl;
}

ConstructNode::ConstructNode(float _size, float _xPos, float _yPos, float _zPos, bool _isLeaf) :
    m_depthSPT(0),
    m_size(_size),
    m_position{_xPos, _yPos, _zPos},
    m_status(NODE_OPEN),
    m_isLeaf(_isLeaf),
    m_nbPoints(0),
    m_vertexIndex(0),
    m_instIndex(0),
    m_instPrecision(0.f),
    m_children{nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr},
    m_outPoints(nullptr),
    m_layerIndexes(nullptr),
    mp_spt(nullptr),
    mp_qspt(nullptr)
{
    m_inPoints = _isLeaf ? new PointXYZRGB[MAX_POINTS_PER_NODE] : nullptr;
//    m_inPoints = _isLeaf ? (PointXYZRGB*)malloc(MAX_POINTS_PER_NODE * sizeof(PointXYZRGB)) : nullptr;
}

//--- Destructors ---//

NestedOctree::~NestedOctree()
{
    delete[] m_vertexData;
    delete[] m_instData;
    delete m_constructRoot;
    delete m_exploitRoot;
}

ConstructNode::~ConstructNode()
{
    for (int i = 0; i < 8; i++) {
        delete m_children[i];
    }
    delete[] m_inPoints;
    delete[] m_outPoints;
    delete[] m_layerIndexes;
    delete mp_spt;
    delete mp_qspt;
}

ExploitNode::~ExploitNode()
{
    for (int i = 0; i < 8; i++) {
        delete m_children[i];
    }
    delete[] m_layerIndexes;
}

//--- Streaming ---//

bool NestedOctree::save(std::ostream& ostream) const
{
    // Init timers
    std::chrono::system_clock::time_point startTime, nodeTime, endTime;
    startTime = std::chrono::system_clock::now();

    ostream.write((char*)&m_format, sizeof(m_format));
    ostream.write((char*)&m_sizePoint, sizeof(m_sizePoint));
    ostream.write((char*)&m_rootSize, sizeof(m_rootSize));
    ostream.write((char*)m_rootPosition, sizeof(m_rootPosition));
    ostream.write((char*)&m_realBBox, sizeof(m_realBBox));
    ostream.write((char*)&m_status, sizeof(m_status));

    ostream.write((char*)&m_nbPoints, sizeof(m_nbPoints));
    ostream.write((char*)&m_nbInnerPoints, sizeof(m_nbInnerPoints));
    ostream.write((char*)&m_nbDiscardedPoints, sizeof(m_nbDiscardedPoints));
    ostream.write((char*)&m_nbLeaves, sizeof(m_nbLeaves));
    ostream.write((char*)&m_nbInnerNodes, sizeof(m_nbInnerNodes));

    ostream.write((char*)&m_nbVertex, sizeof(m_nbVertex));
    ostream.write((char*)&m_nbInst, sizeof(m_nbInst));

    //*** Classic serialization for pointer ***
    if (m_constructRoot != nullptr) {
        m_constructRoot->save(ostream);
    }
    else {
        std::cout << "Error: no node in the octree." << std::endl;
        return false;
    }
    nodeTime = std::chrono::system_clock::now();

    if (m_vertexData != nullptr) {
        ostream.write((char*)m_vertexData, m_nbVertex * m_sizePoint);
        ostream.write((char*)m_instData, m_nbInst * 4 * sizeof(float));
    }
    else {
        std::cout << "Error: no point in the octree." << std::endl;
        return false;
    }
    endTime = std::chrono::system_clock::now();

    float dtAll = std::chrono::duration<float, std::ratio<1>>(endTime - startTime).count();
    float dtNodes = std::chrono::duration<float, std::ratio<1>>(nodeTime - startTime).count();
    float dtPoints = std::chrono::duration<float, std::ratio<1>>(endTime - nodeTime).count();

    std::cout << std::setprecision(3) << "Saving time (s): " << dtAll << " (" << dtNodes << ", " << dtPoints << ")" << std::endl;

    return true;
}

// Constructor that read back the file
NestedOctree::NestedOctree(std::istream &istream, bool _loadData, uint32_t &_sizeofPoint, uint64_t &_pointOffset, uint64_t &_instOffset)
{
    // Init timers
    std::chrono::system_clock::time_point startTime, nodeTime, endTime;
    startTime = std::chrono::system_clock::now();

    istream.read((char*)&m_format, sizeof(m_format));
    istream.read((char*)&m_sizePoint, sizeof(m_sizePoint));
    istream.read((char*)&m_rootSize, sizeof(m_rootSize));
    istream.read((char*)m_rootPosition, sizeof(m_rootPosition));
    istream.read((char*)&m_realBBox, sizeof(m_realBBox));
    istream.read((char*)&m_status, sizeof(m_status));

    istream.read((char*)&m_nbPoints, sizeof(m_nbPoints));
    istream.read((char*)&m_nbInnerPoints, sizeof(m_nbInnerPoints));
    istream.read((char*)&m_nbDiscardedPoints, sizeof(m_nbDiscardedPoints));
    istream.read((char*)&m_nbLeaves, sizeof(m_nbLeaves));
    istream.read((char*)&m_nbInnerNodes, sizeof(m_nbInnerNodes));

    istream.read((char*)&m_nbVertex, sizeof(m_nbVertex));
    istream.read((char*)&m_nbInst, sizeof(m_nbInst));
    std::cout << "Number of points to read: " << m_nbVertex << std::endl;
    std::cout << "Number of instances to read: " << m_nbInst << std::endl;

    //*** Classic unserialization for pointer ***
    m_exploitRoot = ExploitNode::create(istream);
    nodeTime = std::chrono::system_clock::now();

	// Return values
	_sizeofPoint = m_sizePoint;
	_pointOffset = istream.tellg();  // theorically = 77
	std::cout << "Points data start at position " << _pointOffset << " in file" << std::endl;
	_instOffset = _pointOffset + m_nbVertex * m_sizePoint;

	if (_loadData) {
		m_vertexData = new char[m_nbVertex * m_sizePoint];
		if (m_nbInst > 0) {
			m_instData = new char[m_nbInst * 4 * sizeof(float)];
		}
		istream.read((char*)m_vertexData, m_nbVertex * m_sizePoint);
		istream.read((char*)m_instData, m_nbInst * 4 * sizeof(float));
	}
	else {

		m_vertexData = nullptr;
		m_instData = nullptr;
	}
    endTime = std::chrono::system_clock::now();

    float dtAll = std::chrono::duration<float, std::ratio<1>>(endTime - startTime).count();
    float dtNodes = std::chrono::duration<float, std::ratio<1>>(nodeTime - startTime).count();
    float dtPoints = std::chrono::duration<float, std::ratio<1>>(endTime - nodeTime).count();

    std::cout << std::setprecision(3) << "Loading time (s): " << dtAll << " (" << dtNodes << ", " << dtPoints << ")" << std::endl;
}

bool ConstructNode::save(std::ostream& ostream) const
{
    // Store only the arguments that will be reopened in ExploitNode
    ostream.write((char*)&m_depthSPT, sizeof(m_depthSPT));
    ostream.write((char*)&m_size, sizeof(m_size));
    ostream.write((char*)m_position, sizeof(m_position));
    ostream.write((char*)&m_isLeaf, sizeof(m_isLeaf));
    ostream.write((char*)&m_vertexIndex, sizeof(m_vertexIndex));
    ostream.write((char*)&m_instIndex, sizeof(m_instIndex));

    uint8_t isNull = 0x00;
    uint8_t notNull = 0xFF;
    for (unsigned int i = 0; i < 8; i++) {
        if (m_children[i] != nullptr) {
            // Write the tag value for a present child
            ostream.write((char*)&notNull, sizeof(notNull));
            // Save the child
            m_children[i]->save(ostream);
        }
        else {
            // Write a value that means that there is no child
            ostream.write((char*)&isNull, sizeof(isNull));
        }
    }

    ostream.write((char*)m_layerIndexes, (m_depthSPT + 1) * sizeof(uint32_t));

    return true;
}

// Static function
ExploitNode* ExploitNode::create(std::istream& istream)
{
    // create a default ExploitNode
    ExploitNode* node = new ExploitNode();
    istream.read((char*)&node->m_depthSPT, sizeof(m_depthSPT));
    istream.read((char*)&node->m_size, sizeof(m_size));
    istream.read((char*)node->m_position, sizeof(m_position));
    istream.read((char*)&node->m_isLeaf, sizeof(m_isLeaf));
    istream.read((char*)&node->m_vertexIndex, sizeof(m_vertexIndex));
    istream.read((char*)&node->m_instIndex, sizeof(m_instIndex));

    uint8_t v;
    for (unsigned int i = 0; i < 8; i++) {
        istream.read((char*)&v, sizeof(v));
        if (v == 0xFF) {
            node->m_children[i] = ExploitNode::create(istream);
        }
        if (v != 0x00 && v != 0xFF) {
            std::cout << "Warning: wrong tag value for node children: " << v << std::endl;
        }
    }

    node->m_layerIndexes = new uint32_t[node->m_depthSPT + 1];
    istream.read((char*)node->m_layerIndexes, (node->m_depthSPT + 1) * sizeof(uint32_t));

    return node;
}

//--- Functions ---//

void NestedOctree::getBoundingBox(BoundingBox & _bbx)
{
    _bbx = m_realBBox;
#ifdef _DEBUG_
    std::cout << "Bounding box extracted from points coordinnates:" << std::endl
              << "X " << m_realBBox.xMin << " " << m_realBBox.xMax << std::endl
              << "Y " << m_realBBox.yMin << " " << m_realBBox.yMax << std::endl
              << "Z " << m_realBBox.zMin << " " << m_realBBox.zMax << std::endl;
#endif
}

void NestedOctree::getCenter(float& _x, float& _y, float& _z) const
{
	_x = m_rootPosition[0] + m_rootSize / 2.f;
	_y = m_rootPosition[1] + m_rootSize / 2.f;
	_z = m_rootPosition[2] + m_rootSize / 2.f;
}

float NestedOctree::getSize() const
{
	return m_rootSize;
}

ExploitNode* NestedOctree::getRootNode() const
{
	return m_exploitRoot;
}

void NestedOctree::insertPoint(PointXYZRGB const& _point)
{
    // if the NestedOctree is empty, create the first node
    if (m_constructRoot == nullptr) {
        // By default, set the first point as the first point
        m_rootPosition[0] = _point.x - m_rootSize / 2;
        m_rootPosition[1] = _point.y - m_rootSize / 2;
        m_rootPosition[2] = _point.z - m_rootSize / 2;
        m_constructRoot = new ConstructNode(m_rootSize, m_rootPosition[0], m_rootPosition[1], m_rootPosition[2], true);
        m_constructRoot->insertPoint(_point);
        m_realBBox = {_point.x, _point.x, _point.y, _point.y, _point.z, _point.z};
        return;
    }
    else {
        // Test if the point is included in the current bounding box
        if (_point.x >= m_realBBox.xMin &&
            _point.y >= m_realBBox.yMin &&
            _point.z >= m_realBBox.zMin &&
            _point.x <= m_realBBox.xMax &&
            _point.y <= m_realBBox.yMax &&
            _point.z <= m_realBBox.zMax)
        {
            m_nbDiscardedPoints += m_constructRoot->insertPoint(_point);
        }
        else {
            // Extend the bounding box
            m_realBBox.xMin = _point.x < m_realBBox.xMin ? _point.x : m_realBBox.xMin;
            m_realBBox.xMax = _point.x > m_realBBox.xMax ? _point.x : m_realBBox.xMax;
            m_realBBox.yMin = _point.y < m_realBBox.yMin ? _point.y : m_realBBox.yMin;
            m_realBBox.yMax = _point.y > m_realBBox.yMax ? _point.y : m_realBBox.yMax;
            m_realBBox.zMin = _point.z < m_realBBox.zMin ? _point.z : m_realBBox.zMin;
            m_realBBox.zMax = _point.z > m_realBBox.zMax ? _point.z : m_realBBox.zMax;

            // Test if the point is inside root node
            if (_point.x >= m_rootPosition[0] &&
                _point.y >= m_rootPosition[1] &&
                _point.z >= m_rootPosition[2] &&
                _point.x <= m_rootPosition[0] + m_rootSize &&
                _point.y <= m_rootPosition[1] + m_rootSize &&
                _point.z <= m_rootPosition[2] + m_rootSize )
            {
                m_nbDiscardedPoints += m_constructRoot->insertPoint(_point);
            }
            else {
               // Optimal expension
                uint32_t childPos = 0;
                if (_point.x < m_rootPosition[0]) {
                    m_rootPosition[0] -= m_rootSize;
                    childPos += 4;
                }
                if (_point.y < m_rootPosition[1]) {
                    m_rootPosition[1] -= m_rootSize;
                    childPos += 2;
                }
                if (_point.z < m_rootPosition[2]) {
                    m_rootPosition[2] -= m_rootSize;
                    childPos += 1;
                }
                m_rootSize = m_rootSize * 2;

                ConstructNode* newRoot = new ConstructNode(m_rootSize,
                                               m_rootPosition[0],
                                               m_rootPosition[1],
                                               m_rootPosition[2],
                                               false); // not a leaf

                // reference the former root as a child of the new root
                newRoot->setChild(childPos, m_constructRoot);
                m_constructRoot = newRoot;

                std::cout << "Nested Octree : changing root size = " << m_rootSize << std::endl;

                this->insertPoint(_point);
            }
        }
    }
}

DetailOctree NestedOctree::checkOctree()
{
    if (!m_constructRoot) {
        std::cout << "Error: no construct root in octree." << std::endl;
        return DetailOctree();
    }

    // Close all the nodes and get their stats
    DetailOctree result = m_constructRoot->countNodes(0);

    // Store the resulting statistics
    m_nbPoints = result.nbPoints;
    m_nbInnerPoints = result.nbInnerPoints;
    m_nbDiscardedPoints += result.nbDiscardedPoints;
    m_nbLeaves = result.nbLeaves;
    m_nbInnerNodes = result.nbInnerNodes;

#ifdef _DEBUG_
    std::cout << "Points : " << m_nbPoints << ", Inner points : " << m_nbInnerPoints << ", Leaves : " << m_nbLeaves << ", Inner nodes : " << m_nbInnerNodes << ", Discarded points : " << m_nbDiscardedPoints << ", depth of Octree : " << result.maxDepth << std::endl;
    std::cout << std::setprecision(4);
    for (uint16_t i = 0; i < DETAIL_MAX_SPT_DEPTH; i++) {
        std::cout << "SPT with depth " << i << ": " << result.nbSptDepth[i] << " leaves, "
                  << result.nbNodesSptDepth[i] << " nodes"
                  << "   (" << ((float)result.nbPointsSptDepth[i]/result.nbSptDepth[i]) << ")" << std::endl;
    }
#endif

    return result;
}

void NestedOctree::close()
{
    try {
    if (m_status == OCTREE_OPEN) {
        // Init timers
        std::chrono::system_clock::time_point startTime, nodeTime, endTime;
        startTime = std::chrono::system_clock::now();

        switch (m_format) {
        case TL_OCTREE_UNCOMPRESSED:
            m_constructRoot->constructSPT(m_nbDiscardedPoints);
            break;
        case TL_OCTREE_1MM:
            m_constructRoot->constructQSPT16(m_nbDiscardedPoints, 0.001f);
            break;
        case TL_OCTREE_100UM:
            m_constructRoot->constructQSPT16(m_nbDiscardedPoints, 0.0001f);
            break;
        case TL_OCTREE_10UM:
            m_constructRoot->constructQSPT16(m_nbDiscardedPoints, 0.00001f);
            break;
        default:
            break;
        }

        endTime = std::chrono::system_clock::now();
        float chronoSPT = std::chrono::duration<float, std::ratio<1>>(endTime - startTime).count();
        std::cout << std::setprecision(4) << "SPT Construct time (s): " << chronoSPT << std::endl;

        // regroup the informations about the points, leaves, nodes
        checkOctree();
        m_status = OCTREE_SPT_CLOSED;
    }

    // Split the process in 2 parts in case of memory shortage
    if (m_status == OCTREE_SPT_CLOSED) {
#ifdef _DEBUG_
        std::cout << "Allocating " << (m_nbPoints + m_nbInnerPoints) * m_sizePoint << " bytes in memory..." << std::endl;
#endif
        // Allocate a large chunck of memory to regroup all the points
        m_vertexData = new char[(m_nbPoints + m_nbInnerPoints) * m_sizePoint];
        // Store each SPT linearly in the same array.
        if (m_format == TL_OCTREE_UNCOMPRESSED) {
            m_constructRoot->storeSPT(m_vertexData, m_nbVertex);
        }
        else {
            m_instData = new char[(m_nbLeaves + m_nbInnerNodes) * 4 * sizeof(float)];
            m_constructRoot->storeQSPT(m_vertexData, m_nbVertex, m_instData, m_nbInst);
        }
#ifdef _DEBUG_
        std::cout << "Initial number of points (leaf + inner) = " << m_nbPoints + m_nbInnerPoints << std::endl;
        std::cout << "Instance count: " << m_nbInst << std::endl;
        std::cout << "Total points stored in SPT = " << m_nbVertex << std::endl;
#endif

        m_status = OCTREE_SPT_READY_FOR_TRANSFERT;
    }
    }
    catch (std::bad_alloc&){
        std::cerr << "failed to allocate: the program might run short on memory." << std::endl;
    }
}

bool NestedOctree::isFormatQuantized()
{
    return (m_format != TL_OCTREE_UNCOMPRESSED);
}

uint32_t NestedOctree::vertexDataSize()
{
    if (m_status == OCTREE_SPT_READY_FOR_TRANSFERT) {
        return m_nbVertex * m_sizePoint;
    }
    else {
        return 0;
    }
}

const void* NestedOctree::vertexData()
{
    return (void*)m_vertexData;
}

uint32_t NestedOctree::instDataSize()
{
    return m_nbInst * 4 * sizeof(float);
}

const void* NestedOctree::instData()
{
    return (void*)m_instData;
}

bool NestedOctree::isReadyToLoad()
{
    return (m_status == OCTREE_SPT_READY_FOR_TRANSFERT);
}

int ConstructNode::insertPoint(PointXYZRGB const& _point)
{
    if (m_isLeaf) {
        if (m_nbPoints < MAX_POINTS_PER_NODE) {
            // Add the point to the arrays
            m_inPoints[m_nbPoints] = _point;
            m_nbPoints++;
            return 0;
        }
        else {
            int result = splitNode();
            result += insertPoint(_point);
            return result;
        }
    }
    else {  // this is an Inner Node
        // NOTE(Robin) to take the position as the center of the node is slightly faster
        if (_point.x < m_position[0] + m_size / 2) {
            if (_point.y < m_position[1] + m_size / 2) {
                if (_point.z < m_position[2] + m_size / 2) {
                    if (!m_children[0]) createChild(0);
                    return m_children[0]->insertPoint(_point);
                }
                else {
                    if (!m_children[1]) createChild(1);
                    return m_children[1]->insertPoint(_point);
                }
            }
            else {
                if (_point.z < m_position[2] + m_size / 2) {
                    if (!m_children[2]) createChild(2);
                    return m_children[2]->insertPoint(_point);
                }
                else {
                    if (!m_children[3]) createChild(3);
                    return m_children[3]->insertPoint(_point);
                }
            }
        }
        else {
            if (_point.y < m_position[1] + m_size / 2) {
                if (_point.z < m_position[2] + m_size / 2) {
                    if (!m_children[4]) createChild(4);
                    return m_children[4]->insertPoint(_point);
                }
                else {
                    if (!m_children[5]) createChild(5);
                    return m_children[5]->insertPoint(_point);
                }
            }
            else {
                if (_point.z < m_position[2] + m_size / 2) {
                    if (!m_children[6]) createChild(6);
                    return m_children[6]->insertPoint(_point);
                }
                else {
                    if (!m_children[7]) createChild(7);
                    return m_children[7]->insertPoint(_point);
                }
            }
        }
    }
}

void ConstructNode::createChild(uint32_t _index)
{
    if (_index > 7) { return; }

    // Erase previous child if it exists
    delete m_children[_index];

    // Checks if the children exist, if not creates it
    switch (_index)
    {
    // The coordinates are directly linked to the index of the child
    case 0:
        m_children[_index] = new ConstructNode(m_size/2, m_position[0], m_position[1], m_position[2], true);
        break;
    case 1:
        m_children[_index] = new ConstructNode(m_size/2, m_position[0], m_position[1], m_position[2] + m_size / 2, true);
        break;
    case 2:
        m_children[_index] = new ConstructNode(m_size/2, m_position[0], m_position[1] + m_size / 2, m_position[2], true);
        break;
    case 3:
        m_children[_index] = new ConstructNode(m_size/2, m_position[0], m_position[1] + m_size / 2, m_position[2] + m_size / 2, true);
        break;
    case 4:
        m_children[_index] = new ConstructNode(m_size/2, m_position[0] + m_size / 2, m_position[1], m_position[2], true);
        break;
    case 5:
        m_children[_index] = new ConstructNode(m_size/2, m_position[0] + m_size / 2, m_position[1], m_position[2] + m_size / 2, true);
        break;
    case 6:
        m_children[_index] = new ConstructNode(m_size/2, m_position[0] + m_size / 2, m_position[1] + m_size / 2, m_position[2], true);
        break;
    case 7:
        m_children[_index] = new ConstructNode(m_size/2, m_position[0] + m_size / 2, m_position[1] + m_size / 2, m_position[2] + m_size / 2, true);
        break;
    default:
        break;
    }
}

void ConstructNode::setChild(uint32_t _index, ConstructNode* _n)
{
    // Avoid segmentation fault
    if (_index > 7) { return; }
    // Avoid memory leak
    if (m_children[_index] != nullptr) { delete m_children[_index]; }
    // Store the pointer
    m_children[_index] = _n;
}

//int ConstructNode::splitNode(PointXYZRGB const& _point)
int ConstructNode::splitNode()
{
    m_isLeaf = false;

    // the result indicate the number of points that failed at insertion
    int result = 0;
    for (unsigned int i = 0; i < m_nbPoints; i++) {
        result += insertPoint(m_inPoints[i]);
    }

    delete[] m_inPoints;
    m_inPoints = nullptr;
    m_nbPoints = 0;

    return result;
}

DetailOctree ConstructNode::countNodes(uint32_t _depth)
{
    DetailOctree result;
    if (m_isLeaf) {
        // exit the statistics for this leaf
        result.nbPoints = m_nbPoints;
        result.nbLeaves = 1;
        result.maxDepth = _depth;
        if (m_depthSPT < DETAIL_MAX_SPT_DEPTH - 1) {
            result.nbSptDepth[m_depthSPT]++;
            result.nbPointsSptDepth[m_depthSPT] += m_nbPoints;
        }
        else {
            result.nbSptDepth[DETAIL_MAX_SPT_DEPTH - 1]++;
            result.nbPointsSptDepth[DETAIL_MAX_SPT_DEPTH - 1] += m_nbPoints;
        }
    }
    else {
        // Call the function recursively in the child branches
        for (int i = 0; i < 8; i++) {
            if (m_children[i] != nullptr) {
                result += m_children[i]->countNodes(_depth + 1);
            }
        }
        // Count this node as an inner node
        if (m_depthSPT < DETAIL_MAX_SPT_DEPTH - 1) {
            result.nbNodesSptDepth[m_depthSPT]++;
        }
        else {
            result.nbNodesSptDepth[DETAIL_MAX_SPT_DEPTH - 1]++;
        }
        result.nbInnerPoints += m_nbPoints;
        result.nbInnerNodes++;
    }
    return result; 
}

/* Return the final depth of the SPT stored in this node */
uint32_t ConstructNode::constructSPT(uint32_t& _discardedPoints)
{
    if (m_isLeaf && m_status == NODE_OPEN) {
        mp_spt = new SPT(m_nbPoints, m_size, m_position[0], m_position[1], m_position[2], m_inPoints[0]);

        uint32_t discPts = 0;
        // first point already inserted --> start at 1
        for (uint32_t n = 1; n < m_nbPoints; n++) {
            discPts += mp_spt->insertPoint(m_inPoints[n]);
        }

        // free the initial point list
        delete m_inPoints;
        m_inPoints = nullptr;

        // store the new quantized points in a custom array
        m_outPoints = new char[m_nbPoints * sizeof(PointXYZRGB)];

        std::vector<uint32_t> li = mp_spt->constructLayers((PointXYZRGB*)m_outPoints);
        m_depthSPT = li.size() - 1;
        m_layerIndexes = new uint32_t[m_depthSPT + 1];
        memcpy(m_layerIndexes, li.data(), sizeof(uint32_t) * li.size());
        
        m_nbPoints -= discPts;
        m_status = NODE_SPT_CREATED;
        _discardedPoints += discPts;

        // free the temporary SPT construct tree
        delete mp_spt;
        mp_spt = nullptr;
    }
    else {
        // Construct intermediary SPT from the SPT of the children
        // 1. Determine the maximum number of layer among the children
        uint32_t maxChildrenSptDepth = 0;
        for (uint32_t j = 0; j < 8; j++) {
            if (m_children[j] != nullptr) {
                uint32_t childDepth = m_children[j]->constructSPT(_discardedPoints);
                maxChildrenSptDepth = std::max(childDepth, maxChildrenSptDepth);
            }
        }
        // Then truncate to 5 layers for all the inner nodes
        m_depthSPT = std::min(maxChildrenSptDepth, 5u);

        // 2. Calculate the total number of points when reuniting all the partial SPT
        // 2.1 m_depthSPT == 0 we have a SPT with only 1 point
        if (m_depthSPT == 0) {
            m_nbPoints = 1;
            m_outPoints = new char[m_nbPoints * sizeof(PointXYZRGB)];
            m_layerIndexes = new uint32_t[1];
            m_layerIndexes[0] = 1;
            for (uint32_t j = 0; j < 8; j++) {
                if (m_children[j] != nullptr) {
                    m_children[j]->getLayer(0, m_outPoints);
                    m_status = NODE_SPT_CREATED;
                    return m_depthSPT;
                }
            }
        }
        // 2.2 m_depthSPT > 0
        m_nbPoints = 0; 
        for (uint32_t j = 0; j < 8; j++) {
            if (m_children[j] != nullptr) {
                m_nbPoints += m_children[j]->getLayerCount(m_depthSPT - 1);
            }
        }

        // 3. Allocate a new array for all the points and copy the layers - 1
        m_outPoints = new char[m_nbPoints * sizeof(PointXYZRGB)];
        m_layerIndexes = new uint32_t[m_depthSPT + 1];

        // 4. Copy each layer from each child from 0 to m_depthSPT - 1
        uint32_t currentPointIndex = 0;
        for (uint32_t d = 0; d < m_depthSPT; d++) {
            for (uint32_t j = 0; j < 8; j++) {
                if (m_children[j] != nullptr) {
                    // the function getLayer() return the number of points stored
                    currentPointIndex += m_children[j]->getLayer(d, m_outPoints + currentPointIndex * sizeof(PointXYZRGB));
                }
            }
            // for the parent node each layer are incremented
            m_layerIndexes[d + 1] = currentPointIndex;
        }
        if (m_layerIndexes[m_depthSPT] != m_nbPoints) {
            std::cout << "ERROR: Inner SPT generation: theoric number of points : " << m_nbPoints << ", SPT points : " << m_layerIndexes[m_depthSPT] << std::endl;
        }

        // 5. Create the layer 0 which contains only the first point
        m_layerIndexes[0] = 1;

        m_status = NODE_SPT_CREATED;
    }
    // return the SPT depth of this node
    return m_depthSPT;
}

/* Return the final depth of the SPT stored in this node */
uint32_t ConstructNode::constructQSPT16(uint32_t& _discardedPoints, float _precision)
{
    if (m_isLeaf && m_status == NODE_OPEN) {
        if ((m_size / _precision) + 1 > 65535) {
            std::cout << "Construct SPT: node too large for quantization" << std::endl;          
            splitNode();
            // Call the construct SPT function on the same node to continue the recursion
            return constructQSPT16(_discardedPoints, _precision);
        }
        m_instPrecision = _precision;

        uint16_t uiSize = (uint16_t)(m_size / _precision) + 1; // garanted to be <= 65535
        uint16_t uiPosX = (uint16_t)((m_inPoints[0].x - m_position[0]) / _precision);
        uint16_t uiPosY = (uint16_t)((m_inPoints[0].y - m_position[1]) / _precision);
        uint16_t uiPosZ = (uint16_t)((m_inPoints[0].z - m_position[2]) / _precision);
        Coord16 coord = {uiPosX, uiPosY, uiPosZ};
        Color24 color = {m_inPoints[0].r, m_inPoints[0].g, m_inPoints[0].b};
        mp_qspt = new QSPT16(m_nbPoints, uiSize, coord, color);

        uint32_t discPts = 0;
        // first point already inserted --> start at 1
        for (uint32_t n = 1; n < m_nbPoints; n++) {
            uint16_t uiPosX = (uint16_t)((m_inPoints[n].x - m_position[0]) / _precision);
            uint16_t uiPosY = (uint16_t)((m_inPoints[n].y - m_position[1]) / _precision);
            uint16_t uiPosZ = (uint16_t)((m_inPoints[n].z - m_position[2]) / _precision);
            Coord16 coord = {uiPosX, uiPosY, uiPosZ};
            Color24 color = {m_inPoints[n].r, m_inPoints[n].g, m_inPoints[n].b};
            if (uiPosX >= uiSize || uiPosY >= uiSize || uiPosZ >= uiSize) {
                std::cout << "Warning the integral position is too big." << std::endl;
            }
            discPts += mp_qspt->insertPoint(coord, color);
        }
        m_nbPoints -= discPts;

        // free the initial point list
        delete m_inPoints;
        m_inPoints = nullptr;

        // store the new quantized points in a custom array
        m_outPoints = new char[m_nbPoints * SIZE_QUANT_POINT];

        std::vector<uint32_t> li = mp_qspt->constructLayers(m_outPoints);
        m_depthSPT = li.size() - 1;
        m_layerIndexes = new uint32_t[m_depthSPT + 1];
        memcpy(m_layerIndexes, li.data(), sizeof(uint32_t) * li.size());
        
        if (m_nbPoints != m_layerIndexes[m_depthSPT]) {
            std::cout << "Warning: incoherent number of points after QSPT creation" << std::endl;
            std::cout << "Point count = " << m_nbPoints << std::endl;
            std::cout << "Layer count = " << m_layerIndexes[m_depthSPT] << std::endl;
        }

        m_status = NODE_SPT_CREATED;
        _discardedPoints += discPts;

        // free the temporary SPT construct tree
        delete mp_qspt;
        mp_qspt = nullptr;
    }
    else {
        // Construct intermediary SPT from the SPT of the children
        // 0. Set instance precision based on the precision of the leaf
        m_instPrecision = _precision;
        bool reducedPrecision = false;
        while ((m_size / m_instPrecision) + 1 > 65535) {
            m_instPrecision *= 2;
            reducedPrecision = true;
        }

        // 1. Determine the maximum number of layer among the children
        uint32_t maxChildrenSptDepth = 0;
        for (uint32_t j = 0; j < 8; j++) {
            if (m_children[j] != nullptr) {
                uint32_t childDepth = m_children[j]->constructQSPT16(_discardedPoints, _precision);
                maxChildrenSptDepth = std::max(childDepth, maxChildrenSptDepth);
            }
        }
        // Then truncate to 5 layers for all the inner nodes
        m_depthSPT = std::min(maxChildrenSptDepth, 5u);

        // 2. Calculate the total number of points when reuniting all the partial SPT
        // 2.1 m_depthSPT == 0 we have a SPT with only 1 point
        if (m_depthSPT == 0) {
            m_nbPoints = 1;
            m_outPoints = new char[m_nbPoints * SIZE_QUANT_POINT];
            m_layerIndexes = new uint32_t[1];
            m_layerIndexes[0] = 1;
            for (uint32_t j = 0; j < 8; j++) {
                if (m_children[j] != nullptr) {
                    m_children[j]->getQLayer(0, m_outPoints, reducedPrecision, j);
                    m_status = NODE_SPT_CREATED;
                    return m_depthSPT;
                }
            }
        }
        // 2.2 m_depthSPT > 0
        m_nbPoints = 0; 
        for (uint32_t j = 0; j < 8; j++) {
            if (m_children[j] != nullptr) {
                m_nbPoints += m_children[j]->getLayerCount(m_depthSPT - 1);
            }
        }

        // 3. Allocate a new array for all the points and copy the layers - 1
        m_outPoints = new char[m_nbPoints * SIZE_QUANT_POINT];
        m_layerIndexes = new uint32_t[m_depthSPT + 1];

        // 4. Copy each layer from each child from 0 to m_depthSPT - 1
        uint32_t currentPointIndex = 0;
        for (uint32_t d = 0; d < m_depthSPT; d++) {
            for (uint32_t j = 0; j < 8; j++) {
                if (m_children[j] != nullptr) {
                    // the function getQLayer() return the number of points stored
                    currentPointIndex += m_children[j]->getQLayer(d, m_outPoints + currentPointIndex * SIZE_QUANT_POINT, reducedPrecision, j);
                }
            }
            // for the parent node each layer are incremented
            m_layerIndexes[d + 1] = currentPointIndex;
        }
        if (m_layerIndexes[m_depthSPT] != m_nbPoints) {
            std::cout << "ERROR: Inner SPT generation: theoric number of points : " << m_nbPoints << ", SPT points : " << m_layerIndexes[m_depthSPT] << std::endl;
        }

        // 5. Create the layer 0 which contains only the first point
        m_layerIndexes[0] = 1;

        m_status = NODE_SPT_CREATED;
    }
    // return the SPT depth of this node
    return m_depthSPT;
}


uint32_t ConstructNode::getLayerCount(uint32_t _layer)
{
    if (_layer > m_depthSPT) return m_layerIndexes[m_depthSPT];
    else                     return m_layerIndexes[_layer];
}

//uint32_t ConstructNode::getLayer(uint32_t _layer, PointXYZRGB* _p)
uint32_t ConstructNode::getLayer(uint32_t _layer, char* _p)
{
    if (_layer > m_depthSPT) return 0;

    if (_layer == 0) {
        memcpy(_p, m_outPoints, m_layerIndexes[0] * sizeof(PointXYZRGB));
        return m_layerIndexes[0];
    }

    uint32_t layerSize = m_layerIndexes[_layer] - m_layerIndexes[_layer - 1];
    memcpy(_p, m_outPoints + m_layerIndexes[_layer - 1] * sizeof(PointXYZRGB), layerSize * sizeof(PointXYZRGB));
    return layerSize;
}

uint32_t ConstructNode::getQLayer(uint32_t _layer, char* _p, bool _reducedPrecision, uint32_t childPos)
{
    if (_layer > m_depthSPT) return 0;

    if (_layer == 0) {
        memcpy(_p, m_outPoints, m_layerIndexes[0] * SIZE_QUANT_POINT);
        // If the precision has changed between the node, we must rescale the coordinates
        shiftCoords(_p, m_layerIndexes[0], _reducedPrecision, childPos);
        return m_layerIndexes[0];
    }

    uint32_t layerSize = m_layerIndexes[_layer] - m_layerIndexes[_layer - 1];
    memcpy(_p, m_outPoints + m_layerIndexes[_layer - 1] * SIZE_QUANT_POINT, layerSize * SIZE_QUANT_POINT);
    // change the origin of the point based on the child position
    // and rescale the coordinates if the precision has changed
    shiftCoords(_p, layerSize, _reducedPrecision, childPos);
    return layerSize;
}

void ConstructNode::shiftCoords(char* _p, uint32_t count, bool _reducedPrecision, uint32_t childPos)
{
    Coord16 tempCoord;
    for (uint32_t i = 0; i < count; i++) {
        // Copy the data to get aligned struct
        memcpy(&tempCoord, _p + i * SIZE_QUANT_POINT, sizeof(Coord16));

        // We shift the coordinates by the uiSize of the child
        uint16_t shift = (uint16_t)(m_size / m_instPrecision) + 1;

        if (_reducedPrecision) {
            // Shift the coordinates depending on the child and rescale by 2
            switch (childPos) {
            case 0:
                tempCoord.x = tempCoord.x / 2; // ~ tempCoord.x >> 1
                tempCoord.y = tempCoord.y / 2;
                tempCoord.z = tempCoord.z / 2;
                break;
            case 1:
                tempCoord.x = tempCoord.x / 2;
                tempCoord.y = tempCoord.y / 2;
                tempCoord.z = tempCoord.z / 2 + shift; // (z >> 1) | 0x8000
                break;
            case 2:
                tempCoord.x = tempCoord.x / 2;         // (x >> 1) | (childPos & 0x04) << 13
                tempCoord.y = tempCoord.y / 2 + shift; // (y >> 1) | (childPos & 0x02) << 14
                tempCoord.z = tempCoord.z / 2;         // (z >> 1) | (childPos & 0x01) << 15
                break;
            case 3:
                tempCoord.x = tempCoord.x / 2;
                tempCoord.y = tempCoord.y / 2 + shift;
                tempCoord.z = tempCoord.z / 2 + shift;
                break;
            case 4:
                tempCoord.x = tempCoord.x / 2 + shift;
                tempCoord.y = tempCoord.y / 2;
                tempCoord.z = tempCoord.z / 2;
                break;
            case 5:
                tempCoord.x = tempCoord.x / 2 + shift;
                tempCoord.y = tempCoord.y / 2;
                tempCoord.z = tempCoord.z / 2 + shift;
                break;
            case 6:
                tempCoord.x = tempCoord.x / 2 + shift;
                tempCoord.y = tempCoord.y / 2 + shift;
                tempCoord.z = tempCoord.z / 2;
                break;
            case 7:
                tempCoord.x = tempCoord.x / 2 + shift;
                tempCoord.y = tempCoord.y / 2 + shift;
                tempCoord.z = tempCoord.z / 2 + shift;
                break;
            default:
                std::cout << "We should not be here" << std::endl;
                break;
            }
        }
        else {
            // Shift the coordinates depending on the child position in the octree
            switch (childPos) {
            case 0:
                break;
            case 1:
                tempCoord.z += shift;
                break;
            case 2:
                tempCoord.y += shift;
                break;
            case 3:
                tempCoord.y += shift;
                tempCoord.z += shift;
                break;
            case 4:
                tempCoord.x += shift;
                break;
            case 5:
                tempCoord.x += shift;
                tempCoord.z += shift;
                break;
            case 6:
                tempCoord.x += shift;
                tempCoord.y += shift;
                break;
            case 7:
                tempCoord.x += shift;
                tempCoord.y += shift;
                tempCoord.z += shift;
                break;
            default:
                std::cout << "We should not be here" << std::endl;
                break;
            }
        }

        // Copy back the data in the original place
        memcpy(_p + i * SIZE_QUANT_POINT, &tempCoord, sizeof(Coord16));
    }
}

void ConstructNode::storeSPT(char* _vertexData, uint32_t& _vertexIndex)
{
    if (m_status == NODE_SPT_CREATED) {
        memcpy(_vertexData + _vertexIndex * sizeof(PointXYZRGB), m_outPoints, m_layerIndexes[m_depthSPT] * sizeof(PointXYZRGB));
        m_vertexIndex = _vertexIndex;
        _vertexIndex += m_layerIndexes[m_depthSPT];

        delete[] m_outPoints;
        m_outPoints = nullptr;
        m_status = NODE_SPT_STORED;
    }

    for (int j = 0; j < 8; j++) {
        if (m_children[j]) {
            m_children[j]->storeSPT(_vertexData, _vertexIndex);
        }
    }
}

void ConstructNode::storeQSPT(char* _vertexData, uint32_t& _vertexIndex, char* _instData, uint32_t& _instIndex)
{
    if (m_status == NODE_SPT_CREATED) {
        memcpy(_vertexData + _vertexIndex * SIZE_QUANT_POINT, m_outPoints, m_layerIndexes[m_depthSPT] * SIZE_QUANT_POINT);
        m_vertexIndex = _vertexIndex;
        _vertexIndex += m_layerIndexes[m_depthSPT];

        memcpy(_instData + _instIndex * 4 * sizeof(float), m_position, 3 * sizeof(float));
        memcpy(_instData + _instIndex * 4 * sizeof(float) + 3 * sizeof(float), &m_instPrecision, sizeof(float));
        m_instIndex = _instIndex;
        _instIndex++;

        delete[] m_outPoints;
        m_outPoints = nullptr;
        m_status = NODE_SPT_STORED;
    }

    for (int j = 0; j < 8; j++) {
        if (m_children[j]) {
            m_children[j]->storeQSPT(_vertexData, _vertexIndex, _instData, _instIndex);
        }
    }
}

void NestedOctree::getFixedLoD(std::vector<DrawInfos>& _result, float _minPointSpacing)
{
    _result.clear();
    _result.reserve(m_nbLeaves);

    if (m_exploitRoot) {
        m_exploitRoot->getFixedLoD(_result, _minPointSpacing);
    }
}

void NestedOctree::getPerspectiveLoD(std::vector<DrawInfos>& _result, glm::mat4 _viewFrustum, glm::vec3 _cameraPos, uint32_t _screenHeight, float _fovyRad, float _ptSize)
{
    _result.clear();
    // TODO(Robin) Pre-allocate a sufficient space but not based on the number of leaves
    _result.reserve(m_nbLeaves);

    if (m_exploitRoot) {
        m_exploitRoot->getPerspectiveLoD(_result, _viewFrustum, _cameraPos, _screenHeight, _fovyRad, _ptSize);
    }
}

void NestedOctree::getOrthographicLoD(std::vector<DrawInfos>& _result, glm::vec3 _cameraPos, glm::vec4 _wMidPlane, glm::vec4 _hMidPlane, glm::vec4 _zNearPlane, float _width, float _height, float _depth, uint32_t _screenHeight, float _ptSize)
{
    _result.clear();
    _result.reserve(m_nbLeaves);

    if (m_exploitRoot) {
        m_exploitRoot->getOrthographicLoD(_result, _wMidPlane, _hMidPlane, _zNearPlane, _cameraPos, _width, _height, _depth, _screenHeight, _ptSize);
    }
}

void ExploitNode::getFixedLoD(std::vector<DrawInfos>& _result, float _minPointSpacing)
{
    if (m_isLeaf) {
        // truncate the float ratio and add 1Â to get the ceil value
        uint32_t minPointsInLenght = (uint32_t) (m_size / _minPointSpacing) + 1;
        // In our SPT, we have 2^(depth) points in each dimension
        // Now we need to find the minimum depth that get us the minimum points required
        uint32_t maxPointsInLenght = 1;
        uint32_t renderDepth = 0;
        while (maxPointsInLenght < minPointsInLenght) {
            renderDepth++;
            maxPointsInLenght *= 2;  
        }
        // Clamp the depth to the maximum depth of the SPT
        renderDepth = std::min(renderDepth, m_depthSPT);
 //       uint32_t renderDepth = std::min(4u, m_depthSPT);
        // Get the corresponding number of points
        uint32_t pointsToRender = m_layerIndexes[renderDepth];
        _result.push_back({m_vertexIndex, pointsToRender, m_instIndex});
    }

    for (int j = 0; j < 8; j++) {
        if (m_children[j]) {
            m_children[j]->getFixedLoD(_result, _minPointSpacing);
        }
    }
}

void ExploitNode::getPerspectiveLoD(std::vector<DrawInfos>& _result,
                                glm::mat4 _viewFrustum, glm::vec3 _cameraPos,
                                uint32_t _screenHeight, float _fovyRad, float _ptSize)
{
    // First, check if the node is inside the field of view
    glm::vec4 distances = _viewFrustum * glm::vec4(m_position[0] + m_size / 2, m_position[1] + m_size / 2, m_position[2] + m_size / 2, 1.f);
    float radius = m_size / 2 * sqrt(3);
    if (distances[0] < -radius || distances[1] < -radius || distances[2] < -radius || distances[3] < -radius) {
        return;
    }

    // Then, check if the current SPT can provide a LoD sufficient
    // Compute the distance between the node and the camera
    float distance = sqrt(glm::distance2(glm::vec3(m_position[0] + m_size / 2, m_position[1] + m_size / 2, m_position[2] + m_size / 2), _cameraPos));
    // project its size on the screen (between 0 and 1)
    float hScreenSize = m_size / (distance * 2 * tan(_fovyRad / 2.0f));
    // Compute the number of points to get 1 point every 3 pixels in width
    uint32_t minPointsRequired = hScreenSize * _screenHeight / (_ptSize / 2.0);
    // In our SPT, we have 2^depth points in each dimension
    // Now we need to find the minimum depth that get us the minimum points required
    int renderDepth = -1; // we start the test with the maximum layers

    for (int d = m_depthSPT; d >= 0; d--) {
       if ((1u << d) >= minPointsRequired) {
            renderDepth = d;
        }
        else break; // exit loop
    }

    // Verify if a correct depth have been found
    if (renderDepth >= 0) { // && (renderDepth <= m_depthSPT) because of the previous for-loop
        _result.push_back({m_vertexIndex, m_layerIndexes[renderDepth], m_instIndex});
    }
    else if (m_isLeaf) {
        _result.push_back({m_vertexIndex, m_layerIndexes[m_depthSPT], m_instIndex});
    }
    else {
        for (int j = 0; j < 8; j++) {
            if (m_children[j]) {
                m_children[j]->getPerspectiveLoD(_result, _viewFrustum, _cameraPos, _screenHeight, _fovyRad, _ptSize);
            }
        }
    }
}

void ExploitNode::getOrthographicLoD(std::vector<DrawInfos>& _result,
                                glm::vec4 _wMidPlane, glm::vec4 _hMidPlane, glm::vec4 _zNearPlane,
                                glm::vec3 _cameraPos,
                                float _width, float _height, float _depth,
                                uint32_t _screenHeight, float _ptSize)
{
    // First, check if the node is inside the field of view
    float radius = m_size / 2 * sqrt(3);

    float wDistance = glm::dot(_wMidPlane, glm::vec4(m_position[0] + m_size / 2, m_position[1] + m_size / 2, m_position[2] + m_size / 2, 1.f));
    if (wDistance + radius < -(_width / 2) || wDistance - radius > _width / 2) {
        return;
    }
    float hDistance = glm::dot(_hMidPlane, glm::vec4(m_position[0] + m_size / 2, m_position[1] + m_size / 2, m_position[2] + m_size / 2, 1.f));
    if (hDistance + radius < -(_height / 2) || hDistance - radius > _height / 2) {
        return;
    }
    float zDistance = glm::dot(_zNearPlane, glm::vec4(m_position[0] + m_size / 2, m_position[1] + m_size / 2, m_position[2] + m_size / 2, 1.f));
    if (zDistance + radius < -(_depth / 2) || zDistance - radius > _depth / 2) {
        return;
    }

    // Then, check if the current SPT can provide a LoD sufficient
    // Compute the relative size of the Node on the screen
    float hScreenRatio = m_size / _height;
    // Compute the number of points to get minimum 1 point every 1 pixels
    uint32_t minPointsRequired = hScreenRatio * _screenHeight / (_ptSize / 2);
    // In our SPT, we have 2^depth points in each dimension
    // Now we need to find the minimum depth that get us the minimum points required
    int renderDepth = -1; // we start the test with the maximum layers

    for (int d = m_depthSPT; d >= 0; d--) {
        if ((1u << d) >= minPointsRequired) {
            renderDepth = d;
        }
        else break; // exit loop
    }

    // Verify if a correct depth have been found
    if (renderDepth >= 0) { // && (renderDepth <= m_depthSPT) because of the previous for-loop
        _result.push_back({m_vertexIndex, m_layerIndexes[renderDepth], m_instIndex});
    }
    else if (m_isLeaf) {
        _result.push_back({m_vertexIndex, m_layerIndexes[m_depthSPT], m_instIndex});
    }
    else {
        for (int j = 0; j < 8; j++) {
            if (m_children[j]) {
                m_children[j]->getOrthographicLoD(_result, _wMidPlane, _hMidPlane, _zNearPlane, _cameraPos, _width, _height, _depth, _screenHeight, _ptSize);
            }
        }
    }
}
   
uint32_t ExploitNode::getPointCount() const
{
	// The maximum points that contain a node with SPT is the index of the last layer.
	return m_layerIndexes[m_depthSPT];
}

uint32_t ExploitNode::getPointIndex() const
{
	return m_vertexIndex;
}

uint32_t ExploitNode::getInstanceIndex() const
{
	return m_instIndex;
}

ExploitNode* ExploitNode::getChildren(uint32_t _child) const
{
	if (_child < 8) {
		return m_children[_child];
	}
	else {
		return nullptr;
	}
}

bool ExploitNode::isLeaf() const
{
	return m_isLeaf;
}

//------------------------------------//
//     SPT floating-point version    //
//----------------------------------//


SptCell::SptCell(uint32_t d, PointXYZRGB p) :
    depth(d),
    isLeaf(true),
    children{0, 0, 0, 0, 0, 0, 0, 0},
    point(p)
{ }

SPT::SPT(uint32_t _nbPoints, float _size, float _xPos, float _yPos, float _zPos, PointXYZRGB const& _point) :
    m_size(_size),
    m_xPos(_xPos),
    m_yPos(_yPos),
    m_zPos(_zPos),
    m_nbPoints(_nbPoints),
    m_maxDepth(0),
    m_vLayerSizes(std::vector<uint32_t>(1, 1u))
{
    // Allocate the space for the estimate number of cells
    m_vCells.reserve(_nbPoints * 2);
    m_vCells.push_back(SptCell(0, _point));
}

// Wraping functions
uint32_t SPT::insertPoint(PointXYZRGB const& _point)
{
    return insertPoint(0, m_size, m_xPos, m_yPos, m_zPos, _point);
}

uint32_t SPT::insertPoint(uint32_t _id, float _size, float _xPos, float _yPos, float _zPos, PointXYZRGB const& _point)
{
    if (m_vCells[_id].isLeaf) {
        // split the node because a SPT cell can only contain 1 point
        return splitCell(_id, _size, _xPos, _yPos, _zPos, _point);
    }
    else {
        // This cell is an inner node and we must choose where to continue the insertion
        uint32_t childId = 0;

        // We modify directly the value of the parameters to continue the recursion
        if (_point.x >= _xPos + _size / 2) {
            childId += 4;
            _xPos += _size / 2;
        }

        if (_point.y >= _yPos + _size / 2) {
            childId += 2;
            _yPos += _size / 2;
        }

        if (_point.z >= _zPos + _size / 2) {
            childId += 1;
            _zPos += _size / 2;
        }

        // For each 8 position we must verify if the children already exist
        // If there is no children, then we create it and store the PointXYZRGB
        if (m_vCells[_id].children[childId] == 0) {
            m_vCells[_id].children[childId] = m_vCells.size();
            m_vCells.push_back(SptCell(m_vCells[_id].depth + 1, _point));
            m_vLayerSizes[m_vCells[_id].depth + 1]++;
        }
        else {
            return insertPoint(m_vCells[_id].children[childId], _size/2,
                               _xPos, _yPos, _zPos, _point);
        }
        return 0;
    }
}

uint32_t SPT::splitCell(uint32_t _id, float _size, float _xPos, float _yPos, float _zPos, PointXYZRGB const& _point)
{
    if (m_vCells[_id].depth > 12) {
        //std::cout << "High SPT depth, checking point redundancy..." << std::endl;
        if (_point.x == m_vCells[_id].point.x && _point.y == m_vCells[_id].point.y && _point.z == m_vCells[_id].point.z) {
            //std::cout << "Points with same coordinates found" << std::endl;
            return 1;
        }
    }

    m_vCells[_id].isLeaf = false;
    // Increment the max depth of the SPT if we are sure to go beyond
    if (m_vCells[_id].depth >= m_maxDepth) {
        m_maxDepth = m_vCells[_id].depth + 1;
        m_vLayerSizes.push_back(0);
    }
    // the result indicates the number of points that failed at insertion
    // re-insert the point already stored in the node
    insertPoint(_id, _size, _xPos, _yPos, _zPos, m_vCells[_id].point);
    // reduce the next layer point count by 1 to not count it twice
    m_vLayerSizes[m_vCells[_id].depth + 1]--;
    // insert the new point
    return insertPoint(_id, _size, _xPos, _yPos, _zPos, _point);
}

std::vector<uint32_t> SPT::constructLayers(PointXYZRGB* _p)
{
    m_vLayerIndexes.resize(m_maxDepth + 1);
    m_vLayerIndexes[0] = 0;
    for (uint32_t d = 1; d < m_maxDepth + 1; d++) {
        m_vLayerIndexes[d] = m_vLayerIndexes[d - 1] + m_vLayerSizes[d - 1];
    }

    // Fetch recursively a point for each SPTCell (starting at cellId = 0)
    fetchLayers(m_vCells[0], _p, 0, 0);

    return m_vLayerIndexes;
}

void SPT::fetchLayers(SptCell const & _cell, PointXYZRGB* _p, uint32_t _depth, uint32_t _depthLeaf)
{
    if (_cell.isLeaf) {
        _p[m_vLayerIndexes[_depthLeaf]] = _cell.point;
        m_vLayerIndexes[_depthLeaf]++;
    }
    else {
        bool firstChild = true;
        for (uint32_t j = 0; j < 8; j++) {
            if (_cell.children[j] != 0) {
                if (firstChild) {
                    fetchLayers(m_vCells[_cell.children[j]], _p, _depth + 1, _depthLeaf);
                    firstChild = false;
                }
                else fetchLayers(m_vCells[_cell.children[j]], _p, _depth + 1, _depth + 1);
            }
        }
    }
}

//------------------------------------------------------//
//   Quantized SPT with point coordinates on 16 bits   //
//----------------------------------------------------//


QSptCell::QSptCell(uint32_t _d, Coord16 _coord, Color24 _color) :
    depth(_d),
    children{0, 0, 0, 0, 0, 0, 0, 0},
    coord(_coord),
    color(_color),
    isLeaf(true)
{ }

QSPT16::QSPT16(uint32_t _nbPoints, uint16_t _size, Coord16 _ptCoord, Color24 _ptColor) :
    m_size(_size),
    m_nbPoints(_nbPoints),
    m_maxDepth(0),
    m_vLayerSizes(std::vector<uint32_t>(1, 1u))
{
    if (m_size == 0) {
        std::cout << "Error: QSPT size equal 0!" << std::endl;
    }
    // Allocate the space for the estimate number of cells
    m_vCells.reserve(_nbPoints * 2);
    m_vCells.push_back(QSptCell(0, _ptCoord, _ptColor));
}

// Wraping functions
uint32_t QSPT16::insertPoint(Coord16 _ptCoord, Color24 _ptColor)
{
    return insertPoint(0, m_size, 0, 0, 0, _ptCoord, _ptColor);
}

uint32_t QSPT16::insertPoint(uint32_t _id, uint16_t _size, uint16_t _xPos, uint16_t _yPos, uint16_t _zPos, Coord16 _ptCoord, Color24 _ptColor)
{
    if (m_vCells[_id].isLeaf) {
        // split the node and insert the point in the same time
        return splitCell(_id, _size, _xPos, _yPos, _zPos, _ptCoord, _ptColor);
    }
    else {
        if (_ptCoord.x < _xPos + _size/2) {
            if (_ptCoord.y < _yPos + _size/2) {
                if (_ptCoord.z < _zPos + _size/2) {
                    // For each 8 position we must verify if the children already exist
                    // If there is no children, then we create it and store the PointXYZRGB
                    if (m_vCells[_id].children[0] == 0) {
                        m_vCells[_id].children[0] = m_vCells.size();
                        m_vCells.push_back(QSptCell(m_vCells[_id].depth + 1, _ptCoord, _ptColor));
                        m_vLayerSizes[m_vCells[_id].depth + 1]++;
                    }
                    else {
                        return insertPoint(m_vCells[_id].children[0], _size/2,
                                           _xPos,
                                           _yPos,
                                           _zPos, _ptCoord, _ptColor);
                    }
                }
                else {
                    if (m_vCells[_id].children[1] == 0) {
                        m_vCells[_id].children[1] = m_vCells.size();
                        m_vCells.push_back(QSptCell(m_vCells[_id].depth + 1, _ptCoord, _ptColor));
                        m_vLayerSizes[m_vCells[_id].depth + 1]++;
                    }
                    else {
                        return insertPoint(m_vCells[_id].children[1], _size/2,
                                           _xPos,
                                           _yPos,
                                           _zPos + _size/2, _ptCoord, _ptColor);
                    }
                }
            }
            else {
                if (_ptCoord.z < _zPos + _size/2) {
                    if (m_vCells[_id].children[2] == 0) {
                        m_vCells[_id].children[2] = m_vCells.size();
                        m_vCells.push_back(QSptCell(m_vCells[_id].depth + 1, _ptCoord, _ptColor));
                        m_vLayerSizes[m_vCells[_id].depth + 1]++;
                    }
                    else {
                        return insertPoint(m_vCells[_id].children[2], _size/2,
                                           _xPos,
                                           _yPos + _size/2,
                                           _zPos, _ptCoord, _ptColor);
                    }
                }
                else {
                    if (m_vCells[_id].children[3] == 0) {
                        m_vCells[_id].children[3] = m_vCells.size();
                        m_vCells.push_back(QSptCell(m_vCells[_id].depth + 1, _ptCoord, _ptColor));
                        m_vLayerSizes[m_vCells[_id].depth + 1]++;
                    }
                    else {
                        return insertPoint(m_vCells[_id].children[3], _size/2,
                                           _xPos,
                                           _yPos + _size/2,
                                           _zPos + _size/2, _ptCoord, _ptColor);
                    }
                }
            }
        }
        else {
            if (_ptCoord.y < _yPos + _size/2) {
                if (_ptCoord.z < _zPos + _size/2) {
                    if (m_vCells[_id].children[4] == 0) {
                        m_vCells[_id].children[4] = m_vCells.size();
                        m_vCells.push_back(QSptCell(m_vCells[_id].depth + 1, _ptCoord, _ptColor));
                        m_vLayerSizes[m_vCells[_id].depth + 1]++;
                    }
                    else {
                        return insertPoint(m_vCells[_id].children[4], _size/2,
                                           _xPos + _size/2,
                                           _yPos,
                                           _zPos, _ptCoord, _ptColor);
                    }
                }
                else {
                    if (m_vCells[_id].children[5] == 0) {
                        m_vCells[_id].children[5] = m_vCells.size();
                        m_vCells.push_back(QSptCell(m_vCells[_id].depth + 1, _ptCoord, _ptColor));
                        m_vLayerSizes[m_vCells[_id].depth + 1]++;
                    }
                    else {
                        return insertPoint(m_vCells[_id].children[5], _size/2,
                                           _xPos + _size/2,
                                           _yPos,
                                           _zPos + _size/2, _ptCoord, _ptColor);
                    }
                }
            }
            else {
                if (_ptCoord.z < _zPos + _size/2) {
                    if (m_vCells[_id].children[6] == 0) {
                        m_vCells[_id].children[6] = m_vCells.size();
                        m_vCells.push_back(QSptCell(m_vCells[_id].depth + 1, _ptCoord, _ptColor));
                        m_vLayerSizes[m_vCells[_id].depth + 1]++;
                    }
                    else {
                        return insertPoint(m_vCells[_id].children[6], _size/2,
                                           _xPos + _size/2,
                                           _yPos + _size/2,
                                           _zPos, _ptCoord, _ptColor);
                    }
                }
                else {
                    if (m_vCells[_id].children[7] == 0) {
                        m_vCells[_id].children[7] = m_vCells.size();
                        m_vCells.push_back(QSptCell(m_vCells[_id].depth + 1, _ptCoord, _ptColor));
                        m_vLayerSizes[m_vCells[_id].depth + 1]++;
                    }
                    else {
                        return insertPoint(m_vCells[_id].children[7], _size/2,
                                           _xPos + _size/2,
                                           _yPos + _size/2,
                                           _zPos + _size/2, _ptCoord, _ptColor);
                    }
                }
            }
        }
        return 0;
    }
}

uint32_t QSPT16::splitCell(uint32_t _id, uint16_t _size, uint16_t _xPos, uint16_t _yPos, uint16_t _zPos, Coord16 _ptCoord, Color24 _ptColor)
{
    if (_size == 1u) {
        //std::cout << "Points with same coordinates found for the precision used." << std::endl;
        return 1;
    }

    m_vCells[_id].isLeaf = false;
    // Increment the max depth of the SPT if we are sure to go beyond
    if (m_vCells[_id].depth >= m_maxDepth) {
        m_maxDepth = m_vCells[_id].depth + 1;
        m_vLayerSizes.push_back(0);
    }
    // the result indicates the number of points that failed at insertion
    // re-insert the point already stored in the node
    uint32_t result = insertPoint(_id, _size, _xPos, _yPos, _zPos, m_vCells[_id].coord, m_vCells[_id].color);
    // insert the new point
    result += insertPoint(_id, _size, _xPos, _yPos, _zPos, _ptCoord, _ptColor);

    m_vLayerSizes[m_vCells[_id].depth + 1]--;

    return result;
}

std::vector<uint32_t> QSPT16::constructLayers(void* _pData)
{
    m_vLayerIndexes.resize(m_maxDepth + 1);
    m_vLayerIndexes[0] = 0;
    for (uint32_t d = 1; d < m_maxDepth + 1; d++) {
        m_vLayerIndexes[d] = m_vLayerIndexes[d - 1] + m_vLayerSizes[d - 1];
    }

    // Fetch recursively a point for each SPTCell (starting at cellId = 0)
    fetchLayers(m_vCells[0], _pData, 0, 0);

    return m_vLayerIndexes;
}


// TODO(Robin) Try to use a depth offset in the function instead of copy back the point of first child.
void QSPT16::fetchLayers(QSptCell const & _cell, void* _p, uint32_t _depth, uint32_t _depthLeaf)
{
    if (_cell.isLeaf) {
        memcpy((char*)_p + m_vLayerIndexes[_depthLeaf] * SIZE_QUANT_POINT,
               &_cell.coord, sizeof(Coord16));
        memcpy((char*)_p + m_vLayerIndexes[_depthLeaf] * SIZE_QUANT_POINT + sizeof(Coord16),
               &_cell.color, sizeof(Color24));
        m_vLayerIndexes[_depthLeaf]++;
    }
    else {
        bool firstChild = true;
        for (uint32_t j = 0; j < 8; j++) {
            if (_cell.children[j] != 0) {
                // Get the point of the first child
                if (firstChild) {
                    fetchLayers(m_vCells[_cell.children[j]], _p, _depth + 1, _depthLeaf);
                    firstChild = false;
                }
                else fetchLayers(m_vCells[_cell.children[j]], _p, _depth + 1, _depth + 1);
            }
        }
    }
}

// The End
// '''''''
