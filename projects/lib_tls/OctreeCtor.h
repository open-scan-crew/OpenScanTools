#ifndef OCTREE_CTOR_H
#define OCTREE_CTOR_H

#include "OctreeBase.h"
#include "tls_Point.h"

#include <vector>

#define LEAF_DECIMATION 0.0625f
#define NODE_MAX_DEPTH 7u

namespace tls
{
    class QSPT16;

    enum NodeStatus : uint8_t {
        NODE_OPEN,
        NODE_SPT_CREATED,
        NODE_SPT_STORED
    };

    struct TempData
    {
        uint32_t m_nbPoints;
        float m_instPrecision;
        float m_instPosition[3];
        std::vector<tls::Point> m_inPoints;
        char* m_outPoints;
        QSPT16* m_qspt;
    };

    class OctreeCtor : public OctreeBase
    {
    public:
        OctreeCtor(tls::PrecisionType _precisionType, tls::PointFormat ptFormat);
        OctreeCtor(OctreeBase const& base);
        ~OctreeCtor();

        //void insertPointsWithTransfo(Point const* points, uint64_t pointCount, tls::Transformation inTransfo);
        void insertPoint(Point const& _point);
        void insertPoint_overwriteI(Point const& _point);
        void insertPoint_overwriteRGB(Point const& _point);

        // Call encode() when all points have been inserted. No other point can be inserted after !
        void encode(std::ostream& _osResult);

    protected:
        OctreeCtor() {};

        void pushNewCell(float size, float x, float y, float z, bool isLeaf);
        void insertPointInCell(Point const& _point, uint32_t _cellId);
        void createChildForCell(uint32_t _childPos, uint32_t _cellId);
        bool splitCell(uint32_t _cellId);

        // Rearrange the nodes in m_vNodes to get the root node first then
        // the other nodes by depth order (cannot by accomplished during construction)
        //void sortNodes();
        void printStats(std::ostream& _os);
        uint32_t getOctreeDepth(uint32_t _cellId);

        void createLayerForCell(uint32_t cellId);
        void constructQSPTForLeaf(uint32_t _cellId);

        void determineSptDepthFromChildren(uint32_t _cellId);

        void regroupData();
        void storeNodeQSPT(uint32_t _cellId, uint32_t _depth, uint32_t _writeDepth, uint64_t& _dataStoredOffset);
        void storeLeaves(uint32_t _cellId, uint64_t& _dataStoredOffset);
        void storeChildQSPT(uint32_t _cellId, uint32_t _depth, uint32_t _writeDepth, uint64_t& _dataStoredOffset);
        void storeQSPT(uint32_t _cellId, uint64_t& _dataStoredOffset);

    private:
        uint32_t m_octreeDepth;
        uint32_t m_nbDiscardedPoints;
        int64_t m_redundantPointCount;

        // Temporary data - Not saved 
        std::vector<TempData> m_vTempData;
    };

    //------------------------------------------------------//
    //   Quantized SPT with point coordinates on 16 bits   // 
    //----------------------------------------------------//

    struct QSptCell {
        QSptCell(uint32_t d, Coord16 coord, uint8_t i, Color24 color);
        uint32_t depth;
        uint32_t children[8];
        Coord16 coord;
        uint8_t i;
        Color24 color;
        bool isLeaf = true;
    };
    // Size Util = 4 + 32 + 6 + 1 + 3 + 1 = 47 bytes per cell (aligned on 48)

    // Quantized Sequenced Point Tree
    // Sequenced Integer Point Tree ?
    class QSPT16
    {
    public:
        QSPT16(uint32_t _nbPoints, uint32_t _size, tls::PointFormat _ptFormat);

        // Copy function
        bool mergeSPT(uint32_t dstDepth, QSPT16& srcSPT, uint32_t srcDepth, bool shiftX, bool shiftY, bool shiftZ);
        bool scaleDown(uint32_t finalDepth);


        // Wraping function that calls the equivalent protected functions at id = 0
        uint32_t insertPoint(Coord16 _ptCoord, uint8_t i, Color24 _ptColor);

        // write directly the layers via _p and return the number of layers
        uint32_t constructLayers(char** _pData, uint32_t& dataSize, uint32_t& iOffset, uint32_t& rgbOffset, uint32_t* layerIndexes);

    protected:
        uint32_t insertPoint(uint32_t _id, uint32_t _size, uint16_t _xPos, uint16_t _yPos, uint16_t _zPos, Coord16 _ptCoord, uint8_t i, Color24 _ptColor);

        uint32_t splitCell(uint32_t _id, uint32_t _size, uint16_t _xPos, uint16_t _yPos, uint16_t _zPos, Coord16 _ptCoord, uint8_t i, Color24 _ptColor);
        bool scaleDownCell(uint32_t _id, uint32_t finalDepth, uint32_t scaleShift);

        // New functions
        void fetchLayers_XYZ_I(QSptCell const& _cell, Coord16* _pXYZ, uint8_t* _pI, uint32_t _depth, uint32_t _depthLeaf);
        void fetchLayers_XYZ_RGB(QSptCell const& _cell, Coord16* _pXYZ, Color24* _pRGB, uint32_t _depth, uint32_t _depthLeaf);
        void fetchLayers_XYZ_I_RGB(QSptCell const& _cell, Coord16* _pXYZ, uint8_t* _pI, Color24* _pRGB, uint32_t _depth, uint32_t _depthLeaf);

    private:
        tls::PointFormat m_pointFormat;
        uint32_t m_size; // size is the length of the side of the cubic cell
        uint32_t m_nbPoints;

        std::vector<QSptCell> m_vCells;

        uint32_t m_maxDepth;
        uint32_t m_layerSizes[MAX_SPT_DEPTH + 1];
        uint32_t m_writeLayerIndexes[MAX_SPT_DEPTH + 1];
    };
}

#endif // !OCTREE_CTOR_H