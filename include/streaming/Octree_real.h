#ifndef _OCTREE_OLD_H_
#define _OCTREE_OLD_H_

#include <cstdint>
#include <vector>
#include <utility>
#include <fstream>
#include <algorithm>

#include "../../glm/0.9.8.5/glm/glm.hpp"
#define SIZE_QUANT_POINT (sizeof(Coord16) + sizeof(Color24))

const uint32_t MAX_POINTS_PER_NODE = 4092;
const uint32_t DETAIL_MAX_SPT_DEPTH = 12;
const uint32_t MAX_SPT_DEPTH = 12;



struct DetailOctree {
    uint32_t nbPoints = 0;
    uint32_t nbInnerPoints = 0;
    uint32_t nbDiscardedPoints = 0;
    uint32_t nbLeaves = 0;
    uint32_t nbInnerNodes = 0;
    uint32_t maxDepth = 0;
    uint32_t nbSptDepth[DETAIL_MAX_SPT_DEPTH] = {0};
    uint32_t nbNodesSptDepth[DETAIL_MAX_SPT_DEPTH] = {0};
    uint32_t nbPointsSptDepth[DETAIL_MAX_SPT_DEPTH] = {0};

    DetailOctree& operator+= (const DetailOctree& rhs)
    {
        nbPoints += rhs.nbPoints;
        nbInnerPoints += rhs.nbInnerPoints;
        nbDiscardedPoints += rhs.nbDiscardedPoints;
        nbLeaves += rhs.nbLeaves;
        nbInnerNodes += rhs.nbInnerNodes;
        maxDepth = std::max(maxDepth, rhs.maxDepth);
        for (uint16_t i = 0; i < DETAIL_MAX_SPT_DEPTH; i++) {
            nbSptDepth[i] += rhs.nbSptDepth[i];
            nbNodesSptDepth[i] += rhs.nbNodesSptDepth[i];
            nbPointsSptDepth[i] += rhs.nbPointsSptDepth[i];
        }
        return *this;
    }
};

template <class Coord_t, class Color_t>
struct Point {
    //virtual static size_t dataSize() = 0; // Cannot be both static and virtual !
    Coord_t x;
    Coord_t y;
    Coord_t z;
    Color_t color;
    // Color_t can represent: rgb, i, irgb, rgba, ...
};

struct PointXYZRGB {
    float x;
    float y;
    float z;
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct BoundingBox {
    float xMin;
    float xMax;
    float yMin;
    float yMax;
    float zMin;
    float zMax;
};

struct DrawInfos {
    uint32_t firstVertex;
    uint32_t vertexCount;
    uint32_t instIndex;
};

class ConstructNode;
class ExploitNode;
class SPT;
class QSPT16;

enum OctreeFormat {
    TL_OCTREE_UNCOMPRESSED = 0,
    TL_OCTREE_1MM,
    TL_OCTREE_100UM,
    TL_OCTREE_10UM
};

class NestedOctree
{
public:
    NestedOctree();
    NestedOctree(OctreeFormat _format);
	NestedOctree(std::istream &istream, bool _loadData, uint32_t &sizeofPoint, uint64_t &pointOffset, uint64_t &instOffset);
    ~NestedOctree();

    bool save(std::ostream& ostream) const;
    //static NestedOctree* create(std::istream& istream);

    //void setBoundingBox(BoundingBox const & _bbx);
    void getBoundingBox(BoundingBox & _bbx);
	void getCenter(float& _x, float& _y, float& _z) const;
	float getSize() const;
	ExploitNode* getRootNode() const;

    void insertPoint(PointXYZRGB const& _point);
    DetailOctree checkOctree();
    // Call close() when all points have been inserted. No other point can be inserted after !
    void close();

    // Functions usefull for Vulkan implementation
    bool isFormatQuantized();
    uint32_t vertexDataSize();
    const void* vertexData();
    uint32_t instDataSize();
    const void* instData();

    bool isReadyToLoad();
    void getFixedLoD(std::vector<DrawInfos>& _result, float _minPointSpacing);

    void getPerspectiveLoD(std::vector<DrawInfos>& _result, glm::mat4 _viewFrustum, glm::vec3 _cameraPos, uint32_t _screenHeight, float _fovyRad, float _ptSize);

    void getOrthographicLoD(std::vector<DrawInfos>& _result, glm::vec3 _cameraPos, glm::vec4 _wMidPlane, glm::vec4 _hMidPlane, glm::vec4 _zNearPlane, float _width, float _height, float _zFar, uint32_t _screenHeight, float _ptSize);

private:
    OctreeFormat m_format;
    uint32_t m_sizePoint;
    float m_rootSize;
    float m_rootPosition[3];
    // Minimum and maximum coordinates for each axis (clipping box)
    // Practical values modified by insertPoint()
    BoundingBox m_realBBox;

    enum OctreeStatus : uint8_t {
        OCTREE_OPEN,
        OCTREE_SPT_CLOSED,
        OCTREE_SPT_READY_FOR_TRANSFERT
    } m_status;

    // Stats
    uint32_t m_nbPoints;
    uint32_t m_nbInnerPoints;
    uint32_t m_nbDiscardedPoints;
    uint32_t m_nbLeaves;
    uint32_t m_nbInnerNodes;

    uint32_t m_nbVertex;
    uint32_t m_nbInst;
    char* m_vertexData;
    char* m_instData;

    ConstructNode* m_constructRoot;
    ExploitNode* m_exploitRoot;

    //std::vector<AllNode> m_vNodes;
    //uint32_t m_iRootNode;
};

/*************/
// We need 2 types of Node: for construction and for exploitation (rendering, computing)
// ConstructNode:
//
// ExploitNode:
/*************/

class ConstructNode
{
public:
    ConstructNode();
    ConstructNode(float _size, float _xPos, float _yPos, float _zPos, bool _isLeaf);
    ~ConstructNode();

    bool save(std::ostream& ostream) const;

    int insertPoint(PointXYZRGB const& _point);
    int splitNode();
    DetailOctree countNodes(uint32_t _depth);

    // To use only after checking (m_isLeaf == false)
    void createChild(uint32_t _index);
    void setChild(uint32_t _index, ConstructNode* _n);

    // SPT construction functions
    uint32_t constructSPT(uint32_t& _discardedPoints);
    uint32_t constructQSPT16(uint32_t& _discardedPoints, float _precision);
    void storeSPT(char* _vertexData, uint32_t& _vertexIndex);
    void storeQSPT(char* _vertexData, uint32_t& _vertexIndex, char* _instData, uint32_t& _instIndex);

protected:
    uint32_t getLayerCount(uint32_t _layer);
    //uint32_t getLayer(uint32_t layer, PointXYZRGB* _p);
    uint32_t getLayer(uint32_t layer, char* _p);
    uint32_t getQLayer(uint32_t layer, char* _p, bool _reducedPrecision, uint32_t childPos);
    void shiftCoords(char* _p, uint32_t pointCount, bool _reducedPrecision, uint32_t childPos);

private:
    // Size and alignment of data // size | align | end of mem
    uint32_t m_depthSPT;          // 4    | 0     | 4
    float m_size;                 // 4    | 4     | 8
    float m_position[3];          // 12   | 8     | 20    // stored in Octree::m_instData
    enum NodeStatus : uint8_t {
        NODE_OPEN,
        NODE_SPT_CREATED,
        NODE_SPT_STORED
    } m_status;                   // 1    | 20    | 21
    bool m_isLeaf;                // 1    | 21    | 22
    uint32_t m_nbPoints;          // 4    | 24    | 28
    uint32_t m_vertexIndex;       // 4    | 28    | 32
    uint32_t m_instIndex;         // 4    | 32    | 36    // stored in file
    float m_instPrecision;        // 4    | 36    | 40    // stored in Octree::m_instData
    ConstructNode* m_children[8]; // 64   | 40    | 104
    PointXYZRGB* m_inPoints;      // 8    | 104   | 112
    char* m_outPoints;            // 8    | 112   | 120   // stored in Octree::m_vertexData
    uint32_t* m_layerIndexes;     // 8    | 120   | 128   // + 7.7 to 8.1 layers per node
    SPT* mp_spt;                  // 8    | 128   | 136
    QSPT16* mp_qspt;              // 8    | 136   | 144
};

// ExploitNode
class ExploitNode
{
public:
    ExploitNode();
    ~ExploitNode();

    static ExploitNode* create(std::istream& istream);

    // Camera filter fonctions
    void getFixedLoD(std::vector<DrawInfos>& _result, float _minPointSpacing);
    void getPerspectiveLoD(std::vector<DrawInfos>& _result,
                           glm::mat4 _viewFrustum, glm::vec3 _cameraPos,
                           uint32_t _screenHeight, float _fovyRad, float _ptSize);

    void getOrthographicLoD(std::vector<DrawInfos>& _result,
                            glm::vec4 _wMidPlane, glm::vec4 _hMidPlane, glm::vec4 _zNearPlane,
                            glm::vec3 _cameraPos,
                            float _width, float _height, float _zFar,
                            uint32_t _screenHeight, float _ptSize);

	// Access function for the streaming prototype
	uint32_t getPointCount() const;
	uint32_t getPointIndex() const;
	uint32_t getInstanceIndex() const;
	ExploitNode* getChildren(uint32_t _child) const;
	bool isLeaf() const;

private:
    // Informations red from file (immuables)
    uint32_t m_depthSPT;          // 4    | 0     | 4   // equivalent to m_layerCount;
    float m_size;                 // 4    | 4     | 8
    float m_position[3];          // 12   | 8     | 20
    bool m_isLeaf;                // 1    | 20    | 21
    uint32_t m_vertexIndex;       // 4    | 24    | 28
    uint32_t m_instIndex;         // 4    | 28    | 32
    ExploitNode* m_children[8];   // 64   | 32    | 96 
    uint32_t* m_layerIndexes;     // 8    | 96   | 104   // + 7.7 to 8.1 layers per node

    // Informations that might change on execution time when streaming data
    // uint32_t m_bufferID; // depends on the strategy choosen for buffering (one or multiple buffer)
    // uint32_t m_vertexIndexVRAM;
    // uint32_t m_instanceIndexVRAM;
    // uint32_t m_vertexIndexRAM;
    // uint32_t m_instanceIndexRAM;
};

struct SptCell {
    SptCell(uint32_t d, PointXYZRGB p);
    uint32_t depth;
    bool isLeaf = true;
    uint32_t children[8];
    PointXYZRGB point;
};
// Size Util = 4 + 4(1) + 32 + 16 = 56 bytes per cell

// Sequenced Point Tree
class SPT
{
public:
    SPT(uint32_t _nbPoints, float _size, float _xPos, float _yPos, float _zPos, PointXYZRGB const& _point);

    // Wraping function that calls the equivalent protected functions at id = 0
    uint32_t insertPoint(PointXYZRGB const& _point);

    // write directly the layers via _p and return the layers indexes in std::vector
    std::vector<uint32_t> constructLayers(PointXYZRGB* _p);

protected:
    uint32_t insertPoint(uint32_t _id, float _size, float _xPos, float _yPos, float _zPos, PointXYZRGB const& _point);

    uint32_t splitCell(uint32_t _id, float _size, float _xPos, float _yPos, float _zPos, PointXYZRGB const& _point);

    void fetchLayers(SptCell const& _cell, PointXYZRGB* _p, uint32_t _depth, uint32_t _depthLeaf);

private:
    float m_size; // size is the lenght of the side of the cubic cell
    float m_xPos; // 
    float m_yPos; // Position is the minimum coordinates of the cell
    float m_zPos; //
    uint32_t m_nbPoints;

    // Custom vector structure for faster assignation
    std::vector<SptCell> m_vCells;

    uint32_t m_maxDepth;
    std::vector<uint32_t> m_vLayerSizes;
    std::vector<uint32_t> m_vLayerIndexes;
};

//------------------------------------------------------//
//   Quantized SPT with point coordinates on 16 bits   // 
//----------------------------------------------------//

struct Coord16 {
    uint16_t x;
    uint16_t y;
    uint16_t z;
};

struct Color24 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct QSptCell {
    QSptCell(uint32_t d, Coord16 coord, Color24 color);
    uint32_t depth;
    uint32_t children[8];
    Coord16 coord;
    Color24 color;
    bool isLeaf = true;
};
// Size Util = 4 + 32 + 6 + 3 + 1 = 48 bytes per cell

// Quantized Sequenced Point Tree
// Sequenced Integer Point Tree ?
class QSPT16
{
public:
    QSPT16(uint32_t _nbPoints, uint16_t _size, Coord16 _ptCoord, Color24 _ptColor);

    // Wraping function that calls the equivalent protected functions at id = 0
    uint32_t insertPoint(Coord16 _ptCoord, Color24 _ptColor);

    // write directly the layers via _p and return the layers indexes in std::vector
    std::vector<uint32_t> constructLayers(void* _pData);

protected:
    uint32_t insertPoint(uint32_t _id, uint16_t _size, uint16_t _xPos, uint16_t _yPos, uint16_t _zPos, Coord16 _ptCoord, Color24 _ptColor);

    uint32_t splitCell(uint32_t _id, uint16_t _size, uint16_t _xPos, uint16_t _yPos, uint16_t _zPos, Coord16 _ptCoord, Color24 _ptColor);

    void fetchLayers(QSptCell const& _cell, void* _p, uint32_t _depth, uint32_t _depthLeaf);

private:
    uint16_t m_size; // size is the lenght of the side of the cubic cell
    uint32_t m_nbPoints;

    // Custom vector structure for faster assignation
    std::vector<QSptCell> m_vCells;

    uint32_t m_maxDepth;
    std::vector<uint32_t> m_vLayerSizes;
    std::vector<uint32_t> m_vLayerIndexes; // offsets in bytes
};


#endif  // _OCTREE_OLD_H_
