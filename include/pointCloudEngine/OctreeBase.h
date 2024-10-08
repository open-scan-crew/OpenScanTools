#ifndef OCTREE_BASE_H
#define OCTREE_BASE_H

#include "models/pointCloud/TLS.h"
#include "models/pointCloud/PointXYZIRGB.h"
#include "io/imports/TlsReader.h"

#include <cstdint>
#include <vector>
#include <utility>

// TODO - Take in account or remove the memory allocation exception in close()

// TODO - Detect when a coordinate is too high to keep the precision asked by the converter


//const uint32_t MAX_POINTS_PER_NODE = 32768;
//const uint32_t MAX_POINTS_PER_NODE = 8192;
const uint32_t MAX_POINTS_PER_NODE = 16384;
const uint32_t MAX_SPT_DEPTH = 16;

#define NO_CHILD (uint32_t) 0xF00DBEEF


// Sizeof TreeCell = 140
struct TreeCell
{   //                      Used in OctreeRayTracing
    uint32_t m_depthSPT;    //  yes (for point count)
    float m_size;           //  yes
    float m_position[3];    //  yes
    bool m_isLeaf;          //  yes
    uint64_t m_dataOffset;  //  yes (for dl)
    uint32_t m_dataSize;    //  yes (for dl)
    uint32_t m_iOffset;     //  no
    uint32_t m_rgbOffset;   //  no
    uint32_t m_children[8]; //  yes
    uint32_t m_layerIndexes[MAX_SPT_DEPTH];  //  yes (for point count)
};

static inline uint32_t aligned(uint32_t v, uint32_t byteAlign)
{
    return (v + byteAlign - 1) & ~(byteAlign - 1);
}

// New struct for the v0.5
/*
struct CellBase
{
    uint32_t m_pointCount;
    bool m_isLeaf;
    uint32_t m_depthSPT;
    union {
        // Node
        struct {
            uint32_t m_layerIndexes[9];
            uint32_t m_children[8];
        };
        // Leaf
        struct {
            uint32_t m_layerIndexes[17];
        };
    };
};

struct CellGeom
{
    bool m_isLeaf;
    union {
        // Node (48 bytes)
        struct {
            float m_position[3];
            float m_size;
            uint32_t m_children[8];
        };
        // Leaf (24 bytes)
        struct {
            float m_posMin[3];
            float m_posMax[3];
        };
    };
};

struct CellData
{
    uint64_t m_dataOffset;
    uint32_t m_dataSize;
    uint32_t m_iOffset;
    uint32_t m_rgbOffset;
};

struct Leaf
{
    uint32_t m_cellId;
    float m_size;
    float m_position[3];
    uint32_t m_pointCount;
};
*/

class OctreeBase
{
public:
    OctreeBase();
    OctreeBase(const tls::PrecisionType& _precision, const tls::PointFormat& ptFormat);
    ~OctreeBase() {};

    tls::PointFormat getPointFormat() const;
    void setPointFormat(tls::PointFormat format);

	const std::vector<TreeCell>& getData() const;
	int64_t getCellCount() const;
	uint64_t getPointCount() const;


    TLS_READER_DECLARE(OctreeBase);

protected:
    const tls::PrecisionType m_precisionType;
    float m_precisionValue;
    tls::PointFormat m_ptFormat;

    float m_rootSize;
    float m_rootPosition[3];
    float m_maxLeafSize;
    float m_minLeafSize;

    uint64_t m_pointCount;
    uint32_t m_cellCount;
    uint32_t m_uRootCell;
    std::vector<TreeCell> m_vTreeCells;
};

#endif  // !OCTREE_BASE_H