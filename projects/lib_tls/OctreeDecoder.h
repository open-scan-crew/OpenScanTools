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

        bool readPointsFromFile(std::fstream& fs, const uint64_t& pointDataOffset);

        uint32_t getCellCount() const;
        uint32_t getCellPointCount(uint32_t _cell_id) const;

        bool isLeaf(uint32_t _cell_id) const;
        bool getNextPoints(Point* dst_buf, uint64_t dst_size, uint64_t& point_count);
        bool copyCellPoints(uint32_t _cell_id, Point* _dst_buf, uint64_t _dst_size, uint64_t& _dst_offset);

    protected:
        void cleanBuffers();
        void decodeCell(uint32_t _cell_id);

    protected:
        std::vector<char*> m_encodedBuffers;

        uint32_t decoded_cell_ = NO_CHILD;
        std::vector<Point> decoded_points_;

        // Read params
        uint32_t current_cell_ = 0;
        uint32_t current_point_ = 0;
    };
}

#endif // !OCTREE_READER_H