#ifndef OCTREE_DECODER_H
#define OCTREE_DECODER_H

#include "OctreeBase.h"
#include "tls_Point.h"

namespace tls
{
    class OctreeDecoder : public OctreeBase
    {
    public:
        OctreeDecoder();
        ~OctreeDecoder();

        void initBuffers();

        bool readPointsFromFile(std::fstream& fs, const uint64_t& pointDataOffset);
        //bool decode();

        uint32_t getCellCount();
        uint32_t getCellPointCount(uint32_t cellId);

        bool isLeaf(uint32_t cellId) const;
        const Point* getCellPoints(uint32_t cellId, uint64_t& pointCount);
        bool copyCellPoints(uint32_t cellId, Point* dstPoints, uint64_t dstSize, uint64_t& dstOffset);

    protected:

        void cleanBuffers();
        void cleanEncodedBuffers();
        void decodeCell(uint32_t cellId, Point* optionalOutput = nullptr);


    protected:
        std::vector<char*> m_encodedBuffers;
        std::vector<Point*> m_decodedBuffers;

        bool m_isEncodedBuffered;
        bool m_isDecoded;
    };
}

#endif // !OCTREE_READER_H